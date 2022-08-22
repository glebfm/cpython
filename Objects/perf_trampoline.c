/*

Perf trampoline instrumentation
===============================

This file contains instrumentation to allow to associate
calls to the CPython eval loop back to the names of the Python
functions and filename being executed.

Many native performance profilers like the Linux perf tools are
only available to 'see' the C stack when sampling from the profiled
process. This means that if we have the following python code:

    import time
    def foo(n):
        # Some CPU intensive code

    def bar(n):
        foo(n)

    def baz(n):
        bar(n)

    baz(10000000)

A performance profiler that is only able to see native frames will
produce the following backtrace when sampling from foo():

    _PyEval_EvalFrameDefault -----> Evaluation frame of foo()
    _PyEval_Vector
    _PyFunction_Vectorcall
    PyObject_Vectorcall
    call_function

    _PyEval_EvalFrameDefault ------> Evaluation frame of bar()
    _PyEval_EvalFrame
    _PyEval_Vector
    _PyFunction_Vectorcall
    PyObject_Vectorcall
    call_function

    _PyEval_EvalFrameDefault -------> Evaluation frame of baz()
    _PyEval_EvalFrame
    _PyEval_Vector
    _PyFunction_Vectorcall
    PyObject_Vectorcall
    call_function

    ...

    Py_RunMain

Because the profiler is only able to see the native frames and the native
function that runs the evaluation loop is the same (_PyEval_EvalFrameDefault)
then the profiler and any reporter generated by it will not be able to
associate the names of the Python functions and the filenames associated with
those calls, rendering the results useless in the Python world.

To fix this problem, we introduce the concept of a trampoline frame. A
trampoline frame is a piece of code that is unique per Python code object that
is executed before entering the CPython eval loop. This piece of code just
calls the original Python evaluation function (_PyEval_EvalFrameDefault) and
forwards all the arguments received. In this way, when a profiler samples
frames from the previous example it will see;

    _PyEval_EvalFrameDefault -----> Evaluation frame of foo()
    [Jit compiled code 3]
    _PyEval_Vector
    _PyFunction_Vectorcall
    PyObject_Vectorcall
    call_function

    _PyEval_EvalFrameDefault ------> Evaluation frame of bar()
    [Jit compiled code 2]
    _PyEval_EvalFrame
    _PyEval_Vector
    _PyFunction_Vectorcall
    PyObject_Vectorcall
    call_function

    _PyEval_EvalFrameDefault -------> Evaluation frame of baz()
    [Jit compiled code 1]
    _PyEval_EvalFrame
    _PyEval_Vector
    _PyFunction_Vectorcall
    PyObject_Vectorcall
    call_function

    ...

    Py_RunMain

When we generate every unique copy of the trampoline (what here we called "[Jit
compiled code N]") we write the relationship between the compiled code and the
Python function that is associated with it. Every profiler requires this
information in a different format. For example, the Linux "perf" profiler
requires a file in "/tmp/perf-PID.map" (name and location not configurable)
with the following format:

    <compiled code address> <compiled code size> <name of the compiled code>

If this file is available when "perf" generates reports, it will automatically
associate every trampoline with the Python function that it is associated with
allowing it to generate reports that include Python information. These reports
then can also be filtered in a way that *only* Python information appears.

Notice that for this to work, there must be a unique copied of the trampoline
per Python code object even if the code in the trampoline is the same. To
achieve this we have a assembly template in Objects/asm_trampiline.S that is
compiled into the Python executable/shared library. This template generates a
symbol that maps the start of the assembly code and another that marks the end
of the assembly code for the trampoline.  Then, every time we need a unique
trampoline for a Python code object, we copy the assembly code into a mmaped
area that has executable permissions and we return the start of that area as
our trampoline function.

Asking for a mmap-ed memory area for trampoline is very wasteful so we
allocate big arenas of memory in a single mmap call, we populate the entire
arena with copies of the trampoline (this allows us to now have to invalidate
the icache for the instructions in the page) and then we return the next
available chunk every time someone asks for a new trampoline. We keep a linked
list of arenas in case the current memory arena is exhausted and another one is
needed.

For the best results, Python should be compiled with
CFLAGS="-fno-omit-frame-pointer -mno-omit-leaf-frame-pointer"` as this allows
profilers to unwind using only the frame pointer and not on DWARF debug
information (note that as trampilines are dynamically generated there won't be
any DWARF information available for them).
*/

#include "Python.h"
#include "pycore_ceval.h"
#include "pycore_frame.h"
#include "pycore_interp.h"

typedef enum {
    PERF_STATUS_FAILED = -1,  // Perf trampoline is in an invalid state
    PERF_STATUS_NO_INIT = 0,  // Perf trampoline is not initialized
    PERF_STATUS_OK = 1,       // Perf trampoline is ready to be executed
} perf_status_t;

#ifdef _PY_HAVE_PERF_TRAMPOLINE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

typedef PyObject *(*py_evaluator)(PyThreadState *, _PyInterpreterFrame *,
                                  int throwflag);
typedef PyObject *(*py_trampoline)(py_evaluator, PyThreadState *,
                                   _PyInterpreterFrame *, int throwflag);

extern void *_Py_trampoline_func_start;  // Start of the template of the
                                         // assembly trampoline
extern void *
    _Py_trampoline_func_end;  // End of the template of the assembly trampoline

struct code_arena_st {
    char *start_addr;    // Start of the memory arena
    char *current_addr;  // Address of the current trampoline within the arena
    size_t size;         // Size of the memory arena
    size_t size_left;    // Remaining size of the memory arena
    size_t code_size;    // Size of the code of every trampoline in the arena
    struct code_arena_st
        *prev;  // Pointer to the arena  or NULL if this is the first arena.
};

typedef struct code_arena_st code_arena_t;

struct trampoline_api_st {
    trampoline_state_init
        init_state;  // Callback to initialize the trampoline state
    trampoline_state_write
        write_state;  // Callback to register every trampoline being created
    trampoline_state_free free_state;  // Callback to free the trampoline state
    void *state;
};

typedef struct trampoline_api_st trampoline_api_t;

static perf_status_t perf_status = PERF_STATUS_NO_INIT;
static Py_ssize_t extra_code_index = -1;
static code_arena_t *code_arena;
static trampoline_api_t trampoline_api;

static FILE *perf_map_file;
void *
_Py_perf_map_get_file(void)
{
    if (perf_map_file) {
        return perf_map_file;
    }
    char filename[100];
    pid_t pid = getpid();
    // Location and file name of perf map is hard-coded in perf tool.
    // Use exclusive create flag wit nofollow to prevent symlink attacks.
    int flags = O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC;
    snprintf(filename, sizeof(filename) - 1, "/tmp/perf-%jd.map",
             (intmax_t)pid);
    int fd = open(filename, flags, 0600);
    if (fd == -1) {
        perf_status = PERF_STATUS_FAILED;
        PyErr_SetFromErrnoWithFilename(PyExc_OSError, filename);
        _PyErr_WriteUnraisableMsg("Failed to create perf map file", NULL);
        return NULL;
    }
    perf_map_file = fdopen(fd, "w");
    if (!perf_map_file) {
        perf_status = PERF_STATUS_FAILED;
        PyErr_SetFromErrnoWithFilename(PyExc_OSError, filename);
        close(fd);
        _PyErr_WriteUnraisableMsg("Failed to create perf map file handle",
                                  NULL);
        return NULL;
    }
    return perf_map_file;
}

int
_Py_perf_map_close(void *state)
{
    FILE *fp = (FILE *)state;
    if (fp) {
        return fclose(fp);
    }
    perf_map_file = NULL;
    perf_status = PERF_STATUS_NO_INIT;
    return 0;
}

void
_Py_perf_map_write_entry(void *state, const void *code_addr,
                         unsigned int code_size, PyCodeObject *co)
{
    assert(state != NULL);
    FILE *method_file = (FILE *)state;
    const char *entry = PyUnicode_AsUTF8(co->co_qualname);
    if (entry == NULL) {
        _PyErr_WriteUnraisableMsg("Failed to get qualname from code object",
                                  NULL);
        return;
    }
    const char *filename = PyUnicode_AsUTF8(co->co_filename);
    if (filename == NULL) {
        _PyErr_WriteUnraisableMsg("Failed to get filename from code object",
                                  NULL);
        return;
    }
    fprintf(method_file, "%p %x py::%s:%s\n", code_addr, code_size, entry,
            filename);
    fflush(method_file);
}

static int
new_code_arena(void)
{
    // non-trivial programs typically need 64 to 256 kiB.
    size_t mem_size = 4096 * 16;
    assert(mem_size % sysconf(_SC_PAGESIZE) == 0);
    char *memory =
        mmap(NULL,  // address
             mem_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
             -1,  // fd (not used here)
             0);  // offset (not used here)
    if (!memory) {
        PyErr_SetFromErrno(PyExc_OSError);
        _PyErr_WriteUnraisableMsg(
            "Failed to create new mmap for perf trampoline", NULL);
        perf_status = PERF_STATUS_FAILED;
        return -1;
    }
    void *start = &_Py_trampoline_func_start;
    void *end = &_Py_trampoline_func_end;
    size_t code_size = end - start;

    size_t n_copies = mem_size / code_size;
    for (size_t i = 0; i < n_copies; i++) {
        memcpy(memory + i * code_size, start, code_size * sizeof(char));
    }
    // Some systems may prevent us from creating executable code on the fly.
    int res = mprotect(memory, mem_size, PROT_READ | PROT_EXEC);
    if (res == -1) {
        PyErr_SetFromErrno(PyExc_OSError);
        munmap(memory, mem_size);
        _PyErr_WriteUnraisableMsg(
            "Failed to set mmap for perf trampoline to PROT_READ | PROT_EXEC",
            NULL);
        return -1;
    }

    code_arena_t *new_arena = PyMem_RawCalloc(1, sizeof(code_arena_t));
    if (new_arena == NULL) {
        PyErr_NoMemory();
        munmap(memory, mem_size);
        _PyErr_WriteUnraisableMsg("Failed to allocate new code arena struct",
                                  NULL);
        return -1;
    }

    new_arena->start_addr = memory;
    new_arena->current_addr = memory;
    new_arena->size = mem_size;
    new_arena->size_left = mem_size;
    new_arena->code_size = code_size;
    new_arena->prev = code_arena;
    code_arena = new_arena;
    return 0;
}

static void
free_code_arenas(void)
{
    code_arena_t *cur = code_arena;
    code_arena_t *prev;
    code_arena = NULL;  // invalid static pointer
    while (cur) {
        munmap(cur->start_addr, cur->size);
        prev = cur->prev;
        PyMem_RawFree(cur);
        cur = prev;
    }
}

static inline py_trampoline
code_arena_new_code(code_arena_t *code_arena)
{
    py_trampoline trampoline = (py_trampoline)code_arena->current_addr;
    code_arena->size_left -= code_arena->code_size;
    code_arena->current_addr += code_arena->code_size;
    return trampoline;
}

static inline py_trampoline
compile_trampoline(void)
{
    if ((code_arena == NULL) ||
        (code_arena->size_left <= code_arena->code_size)) {
        if (new_code_arena() < 0) {
            return NULL;
        }
    }
    assert(code_arena->size_left <= code_arena->size);
    return code_arena_new_code(code_arena);
}

static PyObject *
py_trampoline_evaluator(PyThreadState *ts, _PyInterpreterFrame *frame,
                        int throw)
{
    if (perf_status == PERF_STATUS_FAILED ||
        perf_status == PERF_STATUS_NO_INIT) {
        goto default_eval;
    }
    PyCodeObject *co = frame->f_code;
    py_trampoline f = NULL;
    _PyCode_GetExtra((PyObject *)co, extra_code_index, (void **)&f);
    if (f == NULL) {
        // This is the first time we see this code object so we need
        // to compile a trampoline for it.
        if (extra_code_index == -1) {
            extra_code_index = _PyEval_RequestCodeExtraIndex(NULL);
        }
        py_trampoline new_trampoline = compile_trampoline();
        if (new_trampoline == NULL) {
            goto default_eval;
        }
        trampoline_api.write_state(trampoline_api.state, new_trampoline,
                                   code_arena->code_size, co);
        _PyCode_SetExtra((PyObject *)co, extra_code_index,
                         (void *)new_trampoline);
        f = new_trampoline;
    }
    assert(f != NULL);
    return f(_PyEval_EvalFrameDefault, ts, frame, throw);
default_eval:
    // Something failed, fall back to the default evaluator.
    return _PyEval_EvalFrameDefault(ts, frame, throw);
}
#endif  // _PY_HAVE_PERF_TRAMPOLINE

int
_PyIsPerfTrampolineActive(void)
{
#ifdef _PY_HAVE_PERF_TRAMPOLINE
    PyThreadState *tstate = _PyThreadState_GET();
    return tstate->interp->eval_frame == py_trampoline_evaluator;
#endif
    return 0;
}

int
_PyPerfTrampoline_SetCallbacks(trampoline_state_init init_state,
                               trampoline_state_write write_state,
                               trampoline_state_free free_state)
{
#ifdef _PY_HAVE_PERF_TRAMPOLINE
    if (trampoline_api.state) {
        Py_FatalError("Trampoline state already initialized");
        return -1;
    }
    trampoline_api.init_state = init_state;
    trampoline_api.write_state = write_state;
    trampoline_api.free_state = free_state;
    trampoline_api.state = NULL;
    perf_status = PERF_STATUS_OK;
#endif
    return 0;
}

int
_PyPerfTrampoline_Init(int activate)
{
#ifdef _PY_HAVE_PERF_TRAMPOLINE
    PyThreadState *tstate = _PyThreadState_GET();
    if (tstate->interp->eval_frame &&
        tstate->interp->eval_frame != py_trampoline_evaluator) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Trampoline cannot be initialized as a custom eval "
                        "frame is already present");
        return -1;
    }
    if (!activate) {
        tstate->interp->eval_frame = NULL;
    }
    else {
        tstate->interp->eval_frame = py_trampoline_evaluator;
        if (new_code_arena() < 0) {
            return -1;
        }
        if (trampoline_api.state == NULL) {
            void *state = trampoline_api.init_state();
            if (state == NULL) {
                return -1;
            }
            trampoline_api.state = state;
        }
        perf_status = PERF_STATUS_OK;
    }
#endif
    return 0;
}

int
_PyPerfTrampoline_Fini(void)
{
#ifdef _PY_HAVE_PERF_TRAMPOLINE
    free_code_arenas();
    trampoline_api.free_state(trampoline_api.state);
#endif
    return 0;
}

PyStatus
_PyPerfTrampoline_AfterFork_Child(void)
{
#ifdef _PY_HAVE_PERF_TRAMPOLINE
    // close file in child.
    trampoline_api.free_state(trampoline_api.state);
#endif
    return PyStatus_Ok();
}
