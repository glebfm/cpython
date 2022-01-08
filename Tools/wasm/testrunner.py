#!/usr/bin/env python3
"""
# Crashers

```
test_asyncgen test_asyncore test_builtin test_contextlib_async test_coroutines
test_ftplib test_httplib test_imaplib test_inspect test_mailbox
test_mmap test_pdb test_poplib test_select test_selectors test_smtpd test_smtplib
test_socket test_sys_settrace test_unittest test_urllib test_urllib2
```

## test___all__

```
worker.js onmessage() captured an uncaught exception: RuntimeError: function signature mismatch
Pthread 0x8a1df0 sent an error! undefined:undefined: function signature mismatch

/python-wasm/cpython/builddir/node/python.js:158
   throw ex;
   ^
Error [RuntimeError]: function signature mismatch
    at module_dealloc (<anonymous>:wasm-function[1883]:0x9a15e)
    at _Py_Dealloc (<anonymous>:wasm-function[1959]:0x9eabe)
    at insertdict (<anonymous>:wasm-function[1624]:0x87ccb)
    at _PyDict_SetItem_Take2 (<anonymous>:wasm-function[1622]:0x8749c)
    at dict_ass_sub (<anonymous>:wasm-function[1708]:0x8e604)
    at PyObject_SetItem (<anonymous>:wasm-function[486]:0x34221)
    at finalize_modules (<anonymous>:wasm-function[3644]:0x1703d0)
    at Py_FinalizeEx (<anonymous>:wasm-function[3641]:0x16f986)
    at Py_RunMain (<anonymous>:wasm-function[4055]:0x191124)
    at pymain_main (<anonymous>:wasm-function[4058]:0x19174d)
Emitted 'error' event on process instance at:
    at emitUnhandledRejectionOrErr (internal/event_target.js:579:11)
    at MessagePort.[nodejs.internal.kHybridDispatch] (internal/event_target.js:403:9)
    at MessagePort.exports.emitMessage (internal/per_context/messageport.js:18:26)
```

## test_io

undef HAVE_LARGEFILE_SUPPORT helps

```
test_large_file_ops (test.test_io.CIOTest) ... Aborted(Assertion failed)
/python-wasm/cpython/builddir/node/python.js:158
   throw ex;
   ^

RuntimeError: Aborted(Assertion failed)
    at abort (/python-wasm/cpython/builddir/node/python.js:1295:10)
    at assert (/python-wasm/cpython/builddir/node/python.js:664:3)
    at Object.get64 (/python-wasm/cpython/builddir/node/python.js:4920:42)
    at ___syscall_ftruncate64 (/python-wasm/cpython/builddir/node/python.js:6031:25)
```

## test_uuid (sometimes)

```
0:00:00 load avg: 0.10 [1/1] test_uuid
worker.js onmessage() captured an uncaught exception: RuntimeError: memory access out of bounds
Pthread 0x8a1738 sent an error! undefined:undefined: memory access out of bounds

/python-wasm/cpython/builddir/node/python.js:158
   throw ex;
   ^
Error [RuntimeError]: memory access out of bounds
    at PyObject_ClearWeakRefs (<anonymous>:wasm-function[2738]:0xf8de7)
    at method_dealloc (<anonymous>:wasm-function[871]:0x50e0d)
    at _Py_Dealloc (<anonymous>:wasm-function[1957]:0x9ea6a)
    at tupledealloc (<anonymous>:wasm-function[2149]:0xa9e21)
    at _Py_Dealloc (<anonymous>:wasm-function[1957]:0x9ea6a)
    at list_dealloc (<anonymous>:wasm-function[1454]:0x762d5)
    at _Py_Dealloc (<anonymous>:wasm-function[1957]:0x9ea6a)
    at _PyFrame_Clear (<anonymous>:wasm-function[3342]:0x1525a2)
    at pop_frame (<anonymous>:wasm-function[3091]:0x13276d)
    at _PyEval_EvalFrameDefault (<anonymous>:wasm-function[3085]:0x1291bb)
Emitted 'error' event on process instance at:
```

```
worker.js onmessage() captured an uncaught exception: RuntimeError: invalid index into function table
Pthread 0x8a1738 sent an error! undefined:undefined: invalid index into function table

/python-wasm/cpython/builddir/node/python.js:158
   throw ex;
   ^
Error [RuntimeError]: invalid index into function table
    at _Py_Dealloc (<anonymous>:wasm-function[1957]:0x9ea6a)
    at tupledealloc (<anonymous>:wasm-function[2149]:0xa9e21)
    at _Py_Dealloc (<anonymous>:wasm-function[1957]:0x9ea6a)
    at code_dealloc (<anonymous>:wasm-function[906]:0x533c3)
    at _Py_Dealloc (<anonymous>:wasm-function[1957]:0x9ea6a)
    at func_clear (<anonymous>:wasm-function[1358]:0x70a34)
    at func_dealloc (<anonymous>:wasm-function[1357]:0x709f6)
    at _Py_Dealloc (<anonymous>:wasm-function[1957]:0x9ea6a)
    at insertdict (<anonymous>:wasm-function[1622]:0x87c7d)
    at _PyDict_SetItem_Take2 (<anonymous>:wasm-function[1620]:0x8744e)
Emitted 'error' event on process instance at:
```

## test_mmap

```
test_flush_return_value (test.test_mmap.MmapTests) ... /python-wasm/cpython/builddir/node/python.js:158
   throw ex;
   ^

TypeError: Cannot read property 'stream_ops' of undefined
    at Object.msync (/python-wasm/cpython/builddir/node/python.js:3017:14)
    at Object.msync (/python-wasm/cpython/builddir/node/python.js:8643:17)
    at Object.doMsync (/python-wasm/cpython/builddir/node/python.js:4823:6)
    at ___syscall_msync (/python-wasm/cpython/builddir/node/python.js:6348:12)
```

## test_select

select(nfds, readfds, writefds, exceptfds, timeout) with execptfds != NULL is not supported.

```
test_errno (test.test_select.SelectTestCase) ... Aborted(Assertion failed: exceptfds not supported)
/python-wasm/cpython/builddir/node/python.js:158
   throw ex;
   ^

RuntimeError: Aborted(Assertion failed: exceptfds not supported)
    at abort (/python-wasm/cpython/builddir/node/python.js:1295:10)
    at assert (/python-wasm/cpython/builddir/node/python.js:664:3)
    at ___syscall__newselect (/python-wasm/cpython/builddir/node/python.js:4929:3)
```

## rest

```
TypeError: Cannot read property 'stream' of undefined
    at Object.accept (/python-wasm/cpython/builddir/node/python.js:5354:12)
    at ___syscall_accept4 (/python-wasm/cpython/builddir/node/python.js:5609:31)
```

"""
import pathlib
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor

SRCDIR = pathlib.Path(__file__).parent.parent.parent.absolute()
TESTDIR = SRCDIR / "Lib" / "test"
TESTCASES = sorted(
    f.stem for f in TESTDIR.glob("test_*")
    if f.name.endswith(".py") or f.is_dir()
)

TIMEOUT = 90

RUNNER = ["node", "--experimental-wasm-threads", "--experimental-wasm-bulk-memory", "./python.js", "-m", "test"]

# see https://bugs.python.org/issue46390
MUSL_ISSUE = {
    "test__locale", "test_c_locale_coercion", "test_cmd_line", "test_gdb",
    "test_locale", "test_os", "test_posix", "test_re",
}

# test cases time out, usually caused by socket + threading issue
TEST_TIMEOUT = {
    "test_httpservers", "test_logging", "test_xmlrpc"
}

result = {
    "crash": {},
    "failed": {},
    "abort": {},
    "musl issue": {},
    "timeout": {},
    "ok": {},
    "skipped": {},
}

def runtest(testcase):
    #if testcase in TEST_TIMEOUT:
    #    return testcase, "timeout", None
    start = time.time()
    print(f"{testcase}: ", file=sys.stderr, end="", flush=True)
    try:
        cp = subprocess.run(RUNNER + [testcase], capture_output=True, timeout=TIMEOUT)
    except subprocess.TimeoutExpired:
        cp = None

    if cp is None:
        state = "timeout"
    elif cp.returncode == 7:
        state = "abort" if b"abort" in cp.stderr else "crash"
    elif cp.returncode:
        state = "musl issue" if testcase in MUSL_ISSUE else "failed"
    elif b"test skipped" in cp.stdout:
        state = "skipped"
    else:
        state = "ok"
    print(f"{state} ({round(time.time() - start, 1)}s)", file=sys.stderr)
    return testcase, state, cp

results = []

max_workers = 1
# max_workers = os.cpu_count()

with ThreadPoolExecutor(max_workers=max_workers) as executor:
    for testcase, state, cp in executor.map(runtest, TESTCASES):
        result[state][testcase] = cp

def pt(state, ok):
    testcases = result[state]
    prefix = "" if ok else "# "
    # print(f"## {'*' * 40}")
    prefix = ""
    print(f"## **{state} ({len(testcases)})**")
    print("``")
    for testcase in sorted(testcases):
        print(f"{prefix}{testcase}", end=" ")
    print("\n``")
    print()

print(f"\n\n## {'*' * 40}")
print("# **Results**")
pt("ok", True)
pt("skipped", True)
pt("musl issue", False)
pt("failed", False)
pt("timeout", False)
pt("crash", False)
pt("abort", False)
