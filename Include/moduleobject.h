
/* Module object interface */

#ifndef Py_MODULEOBJECT_H
#define Py_MODULEOBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_DATA(PyTypeObject) PyModule_Type;

#define PyModule_Check(op) PyObject_TypeCheck((op), &PyModule_Type)
#define PyModule_CheckExact(op) Py_IS_TYPE((op), &PyModule_Type)

#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03030000
PyAPI_FUNC(PyObject *) PyModule_NewObject(
    PyObject *name
    );
#endif
PyAPI_FUNC(PyObject *) PyModule_New(
    const char *name            /* UTF-8 encoded string */
    );
PyAPI_FUNC(PyObject *) PyModule_GetDict(PyObject *);
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03030000
PyAPI_FUNC(PyObject *) PyModule_GetNameObject(PyObject *);
#endif
PyAPI_FUNC(const char *) PyModule_GetName(PyObject *);
Py_DEPRECATED(3.2) PyAPI_FUNC(const char *) PyModule_GetFilename(PyObject *);
PyAPI_FUNC(PyObject *) PyModule_GetFilenameObject(PyObject *);
#ifndef Py_LIMITED_API
PyAPI_FUNC(void) _PyModule_Clear(PyObject *);
PyAPI_FUNC(void) _PyModule_ClearDict(PyObject *);
PyAPI_FUNC(int) _PyModuleSpec_IsInitializing(PyObject *);
#endif
PyAPI_FUNC(PyModuleDef*) PyModule_GetDef(PyObject*);
PyAPI_FUNC(void*) PyModule_GetState(PyObject*);

#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03050000
/* New in 3.5 */
PyAPI_FUNC(PyObject *) PyModuleDef_Init(PyModuleDef*);
PyAPI_DATA(PyTypeObject) PyModuleDef_Type;
#endif

typedef struct PyModuleDef_Base {
  PyObject_HEAD
  PyObject* (*m_init)(void);
  Py_ssize_t m_index;
  PyObject* m_copy;
} PyModuleDef_Base;

#define PyModuleDef_HEAD_INIT {  \
    PyObject_HEAD_INIT(_Py_NULL) \
    _Py_NULL, /* m_init */       \
    0,        /* m_index */      \
    _Py_NULL, /* m_copy */       \
  }

#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03050000
/* New in 3.5 */
struct PyModuleDef_Slot {
    int slot;
    void *value;
};

#define Py_mod_create 1
#define Py_mod_exec 2

#ifndef Py_LIMITED_API
#define _Py_mod_LAST_SLOT 2
#endif

#endif /* New in 3.5 */

struct PyModuleConst_Def;
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03b00000
/* New in 3.12 */
enum _PyModuleConst_type {
    _PyModuleConst_none_type = 1,
    _PyModuleConst_long_type = 2,
    _PyModuleConst_ulong_type = 3,
    _PyModuleConst_bool_type = 4,
    _PyModuleConst_double_type = 5,
    _PyModuleConst_string_type = 6,
    _PyModuleConst_call_type = 7,
};

typedef struct PyModuleConst_Def {
    const char *name;
    enum _PyModuleConst_type type;
    union {
        const char *m_str;
        long m_long;
        unsigned long m_ulong;
        double m_double;
        PyObject* (*m_call)(PyObject *module);
        // unused, included to force alignment and size for future use.
        long long m_long_long;
#ifdef HAVE_LONG_DOUBLE
        long double m_long_double;
#endif
#ifdef HAVE_GCC_UINT128_T
        __uint128_t m_uint128;
#endif
#ifdef HAVE_GCC_FLOAT128_T
        __float128_t m_float128;
#endif
    char __m[16]; // 128 bits
    } value;
} PyModuleConst_Def;

PyAPI_FUNC(int) PyModule_AddConstants(PyObject *, PyModuleConst_Def *);

#define PyModuleConst_None(name) \
    {(name), _PyModuleConst_none_type, {.m_long=0}}
#define PyModuleConst_Long(name, value) \
    {(name), _PyModuleConst_long_type, {.m_long=(value)}}
#define PyModuleConst_ULong(name, value) \
    {(name), _PyModuleConst_ulong_type, {.m_ulong=(value)}}
#define PyModuleConst_Bool(name, value) \
    {(name), _PyModuleConst_bool_type, {.m_long=(value)}}
#define PyModuleConst_Double(name, value) \
    {(name), _PyModuleConst_double_type, {.m_double=(value)}}
#define PyModuleConst_String(name, value) \
    {(name), _PyModuleConst_string_type, {.m_str=(value)}}
#define PyModuleConst_Call(name, value) \
    {(name), _PyModuleConst_call_type, {.m_call=(value)}}

#define PyModuleConst_LongMacro(m) PyModuleConst_Long(#m, m)
#define PyModuleConst_StringMacro(m) PyModuleConst_String(#m, m)

#endif /* New in 3.12 */

struct PyModuleDef {
  PyModuleDef_Base m_base;
  const char* m_name;
  const char* m_doc;
  Py_ssize_t m_size;
  PyMethodDef *m_methods;
  PyModuleDef_Slot *m_slots;
  traverseproc m_traverse;
  inquiry m_clear;
  freefunc m_free;
};


// Internal C API
#ifdef Py_BUILD_CORE
extern int _PyModule_IsExtension(PyObject *obj);
#endif

#ifdef __cplusplus
}
#endif
#endif /* !Py_MODULEOBJECT_H */
