/*
 * Copyright (c) 2008-2010 Stefan Krah. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <Python.h>
#include "longintrepr.h"
#include "pythread.h"
#include "structmember.h"
#include "mpdecimal.h"

#include <stdlib.h>

#include "docstrings.h"
#include "memory.h"
#include "mptypes.h"


#if PY_VERSION_HEX < 0x03000000
  #error "Python versions < 3.0 not supported."
#endif

#if defined(_MSC_VER) && defined (CONFIG_64)
  #define _PyLong_AsMpdSsize PyLong_AsLongLong
  #define _PyLong_FromMpdSsize PyLong_FromSsize_t
#else
  #define _PyLong_AsMpdSsize PyLong_AsLong
  #define _PyLong_FromMpdSsize PyLong_FromLong
#endif

#define Dec_INCREF_TRUE (Py_INCREF(Py_True), Py_True)
#define Dec_INCREF_FALSE (Py_INCREF(Py_False), Py_False)

#define MPD_Float_operation MPD_Not_implemented

#define BOUNDS_CHECK(x, MIN, MAX) x = (x < MIN || MAX < x) ? MAX : x


typedef struct {
	PyObject_HEAD
	mpd_t *dec;
} PyDecObject;

typedef struct {
	PyDictObject dict;
	uint32_t *flags;
} PyDecSignalDictObject;

typedef struct {
	PyObject_HEAD
	mpd_context_t ctx;
	PyObject *traps;
	PyObject *flags;
	int capitals;
} PyDecContextObject;

typedef struct {
	PyObject_HEAD
	PyObject *local;
	PyObject *global;
} PyDecContextManagerObject;


static PyTypeObject PyDec_Type;
static PyTypeObject PyDecSignalDict_Type;
static PyTypeObject PyDecContext_Type;
static PyTypeObject PyDecContextManager_Type;
#define PyDec_CheckExact(v) (Py_TYPE(v) == &PyDec_Type)
#define PyDec_Check(v) PyObject_TypeCheck(v, &PyDec_Type)
#define PyDecSignalDict_Check(v) (Py_TYPE(v) == &PyDecSignalDict_Type)
#define PyDecContext_Check(v) (Py_TYPE(v) == &PyDecContext_Type)
#define DecAddr(v) (((PyDecObject *)v)->dec)
#define SdFlagAddr(v) (((PyDecSignalDictObject *)v)->flags)
#define SdFlags(v) (*((PyDecSignalDictObject *)v)->flags)
#define CtxAddr(v) (&((PyDecContextObject *)v)->ctx)
#define CtxCaps(v) (((PyDecContextObject *)v)->capitals)


#ifdef WITHOUT_THREADS
/* Default module context */
static PyObject *module_context = NULL;
#else
/* Key for thread state dictionary */
static PyObject *tls_context_key = NULL;
#endif

/* Template for creating new thread contexts, calling Context() without
 * arguments and initializing the module_context on first access. */
static PyObject *default_context_template = NULL;
/* Basic and extended context templates */
static PyObject *basic_context_template = NULL;
static PyObject *extended_context_template = NULL;


typedef struct {
	const char *name;
	const char *fqname;
	uint32_t mpd_cond;
	PyObject *dec_cond;
} DecCondMap;

/* Top level Exception; inherits from ArithmeticError */
static PyObject *DecimalException = NULL;

/* Exceptions that correspond to IEEE signals; inherit from DecimalException */
static DecCondMap signal_map[] = {
  {"InvalidOperation", "cdecimal.InvalidOperation", MPD_IEEE_Invalid_operation, NULL},
  {"FloatOperation", "cdecimal.FloatOperation", MPD_Float_operation, NULL},
  {"DivisionByZero", "cdecimal.DivisionByZero", MPD_Division_by_zero, NULL},
  {"Overflow", "cdecimal.Overflow", MPD_Overflow, NULL},
  {"Underflow", "cdecimal.Underflow", MPD_Underflow, NULL},
  {"Subnormal", "cdecimal.Subnormal", MPD_Subnormal, NULL},
  {"Inexact", "cdecimal.Inexact", MPD_Inexact, NULL},
  {"Rounded", "cdecimal.Rounded", MPD_Rounded, NULL},
  {"Clamped", "cdecimal.Clamped", MPD_Clamped, NULL},
  {NULL}
};

/* Exceptions that inherit from InvalidOperation */
static DecCondMap cond_map[] = {
  {"InvalidOperation", "cdecimal.InvalidOperation", MPD_Invalid_operation, NULL},
  {"ConversionSyntax", "cdecimal.ConversionSyntax", MPD_Conversion_syntax, NULL},
  {"DivisionImpossible", "cdecimal.DivisionImpossible", MPD_Division_impossible, NULL},
  {"DivisionUndefined", "cdecimal.DivisionUndefined", MPD_Division_undefined, NULL},
  {"FpuError", "cdecimal.FpuError", MPD_Fpu_error, NULL},
  {"InvalidContext", "cdecimal.InvalidContext", MPD_Invalid_context, NULL},
  {"MallocError", "cdecimal.MallocError", MPD_Malloc_error, NULL},
  {NULL}
};

static const char *dec_signal_string[MPD_NUM_FLAGS] = {
	"Clamped",
	"InvalidOperation",
	"DivisionByZero",
	"InvalidOperation",
	"InvalidOperation",
	"InvalidOperation",
	"Inexact",
	"InvalidOperation",
	"InvalidOperation",
	"InvalidOperation",
	"FloatOperation",
	"Overflow",
	"Rounded",
	"Subnormal",
	"Underflow",
};

static const char *invalid_rounding_err =
"valid values for rounding are:\n\
  [ROUND_CEILING, ROUND_FLOOR, ROUND_UP, ROUND_DOWN,\n\
   ROUND_HALF_UP, ROUND_HALF_DOWN, ROUND_HALF_EVEN,\n\
   ROUND_05UP].";

static const char *invalid_signals_err =
"valid values for signals are:\n\
  [InvalidOperation, FloatOperation, DivisionByZero,\n\
   Overflow, Underflow, Subnormal, Inexact, Rounded,\n\
   Clamped].";

static const char *invalid_flags_err =
"valid values for _flags or _traps are:\n\
  signals:\n\
    [DecIEEEInvalidOperation, DecFloatOperation, DecDivisionByZero,\n\
     DecOverflow, DecUnderflow, DecSubnormal, DecInexact, DecRounded,\n\
     DecClamped]\n\
  conditions which trigger DecIEEEInvalidOperation:\n\
    [DecInvalidOperation, DecConversionSyntax, DecDivisionImpossible,\n\
     DecDivisionUndefined, DecFpuError, DecInvalidContext, DecMallocError]";

static int
value_error_int(const char *mesg)
{
	PyErr_SetString(PyExc_ValueError, mesg);
	return -1;
}

static PyObject *
value_error_ptr(const char *mesg)
{
	PyErr_SetString(PyExc_ValueError, mesg);
	return NULL;
}

static int
runtime_error_int(const char *mesg)
{ /* GCOV_NOT_REACHED */
	PyErr_SetString(PyExc_RuntimeError, mesg); /* GCOV_NOT_REACHED */
	return -1; /* GCOV_NOT_REACHED */
}
#define INTERNAL_ERROR_INT(funcname) \
    return runtime_error_int("internal error in " funcname ".")

static PyObject *
runtime_error_ptr(const char *mesg)
{ /* GCOV_NOT_REACHED */
	PyErr_SetString(PyExc_RuntimeError, mesg); /* GCOV_NOT_REACHED */
	return NULL; /* GCOV_NOT_REACHED */
}
#define INTERNAL_ERROR_PTR(funcname) \
    return runtime_error_ptr("internal error in " funcname ".")

static void
dec_traphandler(mpd_context_t *ctx UNUSED) /* GCOV_NOT_REACHED */
{ /* GCOV_NOT_REACHED */
	return;
}

static PyObject *
flags_as_exception(uint32_t flags)
{
	DecCondMap *cm;

	for (cm = signal_map; cm->name != NULL; cm++) {
		if (flags&cm->mpd_cond) {
			return cm->dec_cond;
		}
	}

	INTERNAL_ERROR_PTR("flags_as_exception"); /* GCOV_NOT_REACHED */
}

static uint32_t
exception_as_flags(PyObject *ex)
{
	DecCondMap *cm;

	for (cm = signal_map; cm->name != NULL; cm++) {
		if (cm->dec_cond == ex) {
			return cm->mpd_cond;
		}
	}

	PyErr_SetString(PyExc_ValueError, invalid_signals_err);
	return UINT32_MAX;
}

static PyObject *
flags_as_list(uint32_t flags)
{
	PyObject *list;
	DecCondMap *cm;

	if ((list = PyList_New(0)) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	for (cm = cond_map; cm->name != NULL; cm++) {
		if (flags&cm->mpd_cond) {
			if (PyList_Append(list, cm->dec_cond) < 0) {
				goto error; /* GCOV_UNLIKELY */
			}
		}
	}
	for (cm = signal_map+1; cm->name != NULL; cm++) {
		if (flags&cm->mpd_cond) {
			if (PyList_Append(list, cm->dec_cond) < 0) {
				goto error; /* GCOV_UNLIKELY */
			}
		}
	}

	return list;

error: /* GCOV_UNLIKELY */
	Py_DECREF(list); /* GCOV_UNLIKELY */
	return NULL; /* GCOV_UNLIKELY */
}

static uint32_t
list_as_flags(PyObject *list)
{
	PyObject *item;
	uint32_t flags, x;
	ssize_t n, j;

	if (!PyList_Check(list)) {
		PyErr_SetString(PyExc_TypeError,
		    "argument must be a list of signals.");
		return UINT32_MAX;
	}

	n = PyList_Size(list);
	flags = 0;
	for (j = 0; j < n; j++) {
		item = PyList_GetItem(list, j);
		if ((x = exception_as_flags(item)) == UINT32_MAX) {
			return UINT32_MAX;
		}
		flags |= x;
	}

	return flags;
}

static int
dict_as_flags(PyObject *val)
{
	PyObject *b;
	DecCondMap *cm;
	uint32_t flags = 0;
	int x;

	if (!PyDict_Check(val)) {
		PyErr_SetString(PyExc_TypeError,
		    "argument must be a signal dict.");
		return -1;
	}

	for (cm = signal_map; cm->name != NULL; cm++) {
		if ((b = PyDict_GetItem(val, cm->dec_cond)) == NULL) {
			PyErr_SetString(PyExc_ValueError,
			    "incomplete signal dict.");
			return UINT32_MAX;
		}

		if ((x = PyObject_IsTrue(b)) < 0) {
			return UINT32_MAX; /* GCOV_UNLIKELY */
		}
		if (x == 1) {
			flags |= cm->mpd_cond;
		}
	}

	return flags;
}

static uint32_t
PyLong_AsMpdFlags(PyObject *v)
{
	long x;

	x = PyLong_AsLong(v);
	if (PyErr_Occurred()) {
		return UINT32_MAX;
	}
	if (x < 0 || x > (long)MPD_Max_status) {
		PyErr_SetString(PyExc_ValueError, invalid_flags_err);
		return UINT32_MAX;
	}

	return x;
}

static mpd_ssize_t
PyLong_AsMpdSsize(PyObject *v)
{
	mpd_ssize_t x;

	if (!PyLong_Check(v)) {
		PyErr_SetString(PyExc_TypeError,
		    "integer argument required.");
		return MPD_SSIZE_MAX;
	}

	x = _PyLong_AsMpdSsize(v);
	if (PyErr_Occurred()) {
		return MPD_SSIZE_MAX;
	}

	return x;
}

static int
dec_addstatus(mpd_context_t *ctx, uint32_t status)
{
	ctx->status |= status;
	if (ctx->traps&status) {
		PyObject *ex, *siglist;

		ex = flags_as_exception(ctx->traps&status);
		if (ex == NULL) {
			return 1; /* GCOV_NOT_REACHED */
		}
		siglist = flags_as_list(ctx->traps&status);
		if (siglist == NULL) {
			return 1; /* GCOV_UNLIKELY */
		}

		PyErr_SetObject(ex, siglist);
		Py_DECREF(siglist);
		return 1;
	}
	return 0;
}


/******************************************************************************/
/*                            SignalDict Object                               */
/******************************************************************************/

static int
signaldict_init(PyObject *self, PyObject *args, PyObject *kwds)
{
	if (PyDict_Type.tp_init(self, args, kwds) < 0) {
		return -1; /* GCOV_UNLIKELY */
	}

	SdFlagAddr(self) = NULL;
	return 0;
}

/* sync flags and dictionary, using the flags as the master */
static void
signaldict_update(PyObject *self)
{
	PyObject *b;
	DecCondMap *cm;
	uint32_t flags;

	flags = SdFlags(self);

	for (cm = signal_map; cm->name != NULL; cm++) {
		b = (flags&cm->mpd_cond) ? Py_True : Py_False;
		PyDict_SetItem(self, cm->dec_cond, b);
	}
}

/* set all flags to false */
static int
signaldict_clear_all(PyObject *self)
{
	DecCondMap *cm;

	SdFlags(self) = 0;

	for (cm = signal_map; cm->name != NULL; cm++) {
		if (PyDict_SetItem(self, cm->dec_cond, Py_False) < 0) {
			return -1; /* GCOV_UNLIKELY */
		}
	}
	return 0;
}

static int
signaldict_setitem(PyObject *self, PyObject *key, PyObject *value)
{
	uint32_t flag;
	int x;

	if ((flag = exception_as_flags(key)) == UINT_MAX) {
		return -1;
	}

	if ((x = PyObject_IsTrue(value)) < 0) {
		return -1; /* GCOV_UNLIKELY */
	}
	if (x == 1) {
		SdFlags(self) |= flag;
		PyDict_SetItem(self, key, Py_True);
		return 0;
	}
	else {
		SdFlags(self) &= ~flag;
		PyDict_SetItem(self, key, Py_False);
		return 0;
	}
}

static PyObject *
signaldict_call_unary(PyObject *self, char *name)
{
	signaldict_update(self);
	return PyObject_CallMethod((PyObject *)&PyDict_Type, name, "O", self);
}

static PyObject *
signaldict_richcompare(PyObject *a, PyObject *b, int op)
{
	if (PyDecSignalDict_Check(a)) {
		signaldict_update(a);
	}
	if (PyDecSignalDict_Check(b)) {
		signaldict_update(b);
	}
	return PyDict_Type.tp_richcompare(a, b, op);
}

static int
signaldict_contains(PyObject *self, PyObject *key)
{
	signaldict_update(self);
	return PyDict_Contains(self, key);
}

static PyObject *
signaldict_copy(PyObject *self)
{
	signaldict_update(self);
	return PyDict_Copy(self);
}

static PyObject *
signaldict_get(PyObject *self, PyObject *args)
{
	PyObject *key = NULL, *failobj = NULL;
	if (!PyArg_ParseTuple(args, "O|O", &key, &failobj)) {
		return NULL; /* GCOV_NOT_REACHED (why?) */
	}
	signaldict_update(self);
	if (failobj) {
		return PyObject_CallMethod((PyObject *)&PyDict_Type, "get",
		                           "OOO", self, key, failobj);
	}
	return PyObject_CallMethod((PyObject *)&PyDict_Type, "get",
	                           "OO", self, key);
}

static PyObject *
signaldict_has_key(PyObject *self, PyObject *key)
{
	int ret;
	signaldict_update(self);
	ret = PyDict_Contains(self, key);
	return ret < 0 ? NULL : PyBool_FromLong(ret);
}

static PyObject *
signaldict_items(PyObject *self)
{
	signaldict_update(self);
	return PyDict_Items(self);
}

static PyObject *
signaldict_iter(PyObject *self)
{
	signaldict_update(self);
	return PyDict_Type.tp_iter(self);
}

static PyObject *
signaldict_keys(PyObject *self)
{
	signaldict_update(self);
	return PyDict_Keys(self);
}

static Py_ssize_t
signaldict_length(PyObject *self)
{
	signaldict_update(self);
	return PyDict_Type.tp_as_mapping->mp_length(self);
}

static int
signaldict_print(PyObject *self, FILE *fp, int flags) /* GCOV_UNLIKELY */
{ /* GCOV_UNLIKELY */
	signaldict_update(self); /* GCOV_UNLIKELY */
	return PyDict_Type.tp_print(self, fp, flags); /* GCOV_UNLIKELY */
}

static PyObject *
signaldict_repr(PyObject *self)
{
	signaldict_update(self);
	return PyDict_Type.tp_repr(self);
}

static PyObject *
signaldict_sizeof(PyObject *self)
{
	return signaldict_call_unary(self, "__sizeof__");
}

static int
signaldict_ass_sub(PyObject *self, PyObject *v, PyObject *w)
{
	if (w == NULL) {
		return value_error_int("signal keys cannot be deleted.");
	}
	else {
		return signaldict_setitem(self, v, w);
	}
}

static PyObject *
signaldict_subscript(PyObject *self, PyObject *key)
{
	signaldict_update(self);
	return PyDict_Type.tp_as_mapping->mp_subscript(self, key);
}

static PyObject *
signaldict_values(PyObject *self)
{
	signaldict_update(self);
	return PyDict_Values(self);
}


static PyMappingMethods signaldict_as_mapping = {
	(lenfunc)signaldict_length,        /*mp_length*/
	(binaryfunc)signaldict_subscript,  /*mp_subscript*/
	(objobjargproc)signaldict_ass_sub  /*mp_ass_subscript*/
};

static PySequenceMethods signaldict_as_sequence = {
	0,                      /* sq_length */
	0,                      /* sq_concat */
	0,                      /* sq_repeat */
	0,                      /* sq_item */
	0,                      /* sq_slice */
	0,                      /* sq_ass_item */
	0,                      /* sq_ass_slice */
	signaldict_contains,    /* sq_contains */
	0,                      /* sq_inplace_concat */
	0,                      /* sq_inplace_repeat */
};

static PyMethodDef mapp_methods[] = {
  {"__contains__", (PyCFunction)signaldict_contains,   METH_O|METH_COEXIST, NULL},
  {"__getitem__",  (PyCFunction)signaldict_subscript,  METH_O|METH_COEXIST, NULL},
  {"__sizeof__",   (PyCFunction)signaldict_sizeof,     METH_NOARGS,         NULL},
  {"has_key",      (PyCFunction)signaldict_has_key,    METH_O,              NULL},
  {"get",          (PyCFunction)signaldict_get,        METH_VARARGS,        NULL},
  {"keys",         (PyCFunction)signaldict_keys,       METH_NOARGS,         NULL},
  {"items",        (PyCFunction)signaldict_items,      METH_NOARGS,         NULL},
  {"values",       (PyCFunction)signaldict_values,     METH_NOARGS,         NULL},
  {"copy",         (PyCFunction)signaldict_copy,       METH_NOARGS,         NULL},
  {NULL, NULL}
};

static PyTypeObject PyDecSignalDict_Type =
{
	PyVarObject_HEAD_INIT(0, 0)
	"cdecimal.SignalDict",                    /* tp_name */
	sizeof(PyDecSignalDictObject),            /* tp_basicsize */
	0,                                        /* tp_itemsize */
	0,                                        /* tp_dealloc */
	(printfunc)signaldict_print,              /* tp_print */
	(getattrfunc) 0,                          /* tp_getattr */
	(setattrfunc) 0,                          /* tp_setattr */
	0,                                        /* tp_reserved */
	(reprfunc) signaldict_repr,               /* tp_repr */
	0,                                        /* tp_as_number */
	&signaldict_as_sequence,                  /* tp_as_sequence */
	&signaldict_as_mapping,                   /* tp_as_mapping */
	(hashfunc) PyObject_HashNotImplemented,   /* tp_hash */
	0,                                        /* tp_call */
	(reprfunc) 0,                             /* tp_str */
	(getattrofunc) PyObject_GenericGetAttr,   /* tp_getattro */
	(setattrofunc) 0,                         /* tp_setattro */
	(PyBufferProcs *) 0,                      /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                       /* tp_flags */
	0,                                        /* tp_doc */
	0,                                        /* tp_traverse */
	0,                                        /* tp_clear */
	signaldict_richcompare,                   /* tp_richcompare */
	0,                                        /* tp_weaklistoffset */
	(getiterfunc)signaldict_iter,             /* tp_iter */
	0,                                        /* tp_iternext */
	mapp_methods,                             /* tp_methods */
	0,                                        /* tp_members */
	0,                                        /* tp_getset */
	0,                                        /* tp_base */
	0,                                        /* tp_dict */
	0,                                        /* tp_descr_get */
	0,                                        /* tp_descr_set */
	0,                                        /* tp_dictoffset */
	(initproc)signaldict_init,                /* tp_init */
};


/******************************************************************************/
/*                         Context Object, Part 1                             */
/******************************************************************************/

static PyObject *
context_getprec(PyObject *self, void *closure UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue(CONV_mpd_ssize_t, mpd_getprec(ctx));
}

static PyObject *
context_getemax(PyObject *self, void *closure UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue(CONV_mpd_ssize_t, mpd_getemax(ctx));
}

static PyObject *
context_getemin(PyObject *self, void *closure UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue(CONV_mpd_ssize_t, mpd_getemin(ctx));
}

static PyObject *
context_getetiny(PyObject *self, PyObject *dummy UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue(CONV_mpd_ssize_t, mpd_etiny(ctx));
}

static PyObject *
context_getetop(PyObject *self, PyObject *dummy UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue(CONV_mpd_ssize_t, mpd_etop(ctx));
}

static PyObject *
context_getround(PyObject *self, void *closure UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue("i", mpd_getround(ctx));
}

static PyObject *
context_getcapitals(PyObject *self, void *closure UNUSED)
{
	return Py_BuildValue("i", CtxCaps(self));
}

static PyObject *
context_gettraps(PyObject *self, void *closure UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue("i", mpd_gettraps(ctx));
}

static PyObject *
context_getstatus(PyObject *self, void *closure UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue("i", mpd_getstatus(ctx));
}

static PyObject *
context_getclamp(PyObject *self, void *closure UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue("i", mpd_getclamp(ctx));
}

static PyObject *
context_getallcr(PyObject *self, void *closure UNUSED)
{
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	return Py_BuildValue("i", mpd_getcr(ctx));
}

static int
context_setprec(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_context_t *ctx;
	mpd_ssize_t x;

	x = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsetprec(ctx, x)) {
		return value_error_int(
		    "valid range for prec is [0, MAX_PREC].");
	}

	return 0;
}

static int
context_setemin(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_context_t *ctx;
	mpd_ssize_t x;

	x = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsetemin(ctx, x)) {
		return value_error_int(
		    "valid range for Emin is [MIN_EMIN, 0].");
	}

	return 0;
}

static int
context_setemax(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_context_t *ctx;
	mpd_ssize_t x;

	x = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsetemax(ctx, x)) {
		return value_error_int(
		    "valid range for Emax is [0, MAX_EMAX].");
	}

	return 0;
}

static PyObject *
context_unsafe_setprec(PyObject *self, PyObject *value)
{
	mpd_context_t *ctx = CtxAddr(self);

	ctx->prec = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *
context_unsafe_setemin(PyObject *self, PyObject *value)
{
	mpd_context_t *ctx = CtxAddr(self);

	ctx->emin = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *
context_unsafe_setemax(PyObject *self, PyObject *value)
{
	mpd_context_t *ctx = CtxAddr(self);

	ctx->emax = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return NULL;
	}

	Py_RETURN_NONE;
}

static int
context_setround(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_context_t *ctx;
	mpd_ssize_t x;

	x = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return -1;
	}
	BOUNDS_CHECK(x, INT_MIN, INT_MAX);

	ctx = CtxAddr(self);
	if (!mpd_qsetround(ctx, (int)x)) {
		return value_error_int(invalid_rounding_err);
	}

	return 0;
}

static int
context_setcapitals(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_ssize_t x;

	x = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return -1;
	}
	BOUNDS_CHECK(x, INT_MIN, INT_MAX);

	if (x != 0 && x != 1) {
		return value_error_int(
		    "valid values for capitals are 0 or 1.");
	}
	CtxCaps(self) = (int)x;

	return 0;
}

static int
context_settraps(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_context_t *ctx;
	uint32_t flags;

	flags = PyLong_AsMpdFlags(value);
	if (flags == UINT32_MAX) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsettraps(ctx, flags)) {
		INTERNAL_ERROR_INT("context_settraps"); /* GCOV_NOT_REACHED */
	}

	return 0;
}

static int
context_settraps_list(PyObject *self, PyObject *value)
{
	mpd_context_t *ctx;
	uint32_t flags;

	flags = list_as_flags(value);
	if (flags == UINT32_MAX) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsettraps(ctx, flags)) {
		INTERNAL_ERROR_INT("context_settraps_list"); /* GCOV_NOT_REACHED */
	}

	return 0;
}

static int
context_settraps_dict(PyObject *self, PyObject *value)
{
	mpd_context_t *ctx;
	uint32_t flags;

	flags = dict_as_flags(value);
	if (flags == UINT32_MAX) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsettraps(ctx, flags)) {
		INTERNAL_ERROR_INT("context_settraps_dict"); /* GCOV_NOT_REACHED */
	}

	return 0;
}

static int
context_setstatus(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_context_t *ctx;
	uint32_t flags;

	flags = PyLong_AsMpdFlags(value);
	if (flags == UINT32_MAX) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsetstatus(ctx, flags)) {
		INTERNAL_ERROR_INT("context_setstatus"); /* GCOV_NOT_REACHED */
	}

	return 0;
}

static int
context_setstatus_list(PyObject *self, PyObject *value)
{
	mpd_context_t *ctx;
	uint32_t flags;

	flags = list_as_flags(value);
	if (flags == UINT32_MAX) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsetstatus(ctx, flags)) {
		INTERNAL_ERROR_INT("context_setstatus_list"); /* GCOV_NOT_REACHED */
	}

	return 0;
}

static int
context_setstatus_dict(PyObject *self, PyObject *value)
{
	mpd_context_t *ctx;
	uint32_t flags;

	flags = dict_as_flags(value);
	if (flags == UINT32_MAX) {
		return -1;
	}

	ctx = CtxAddr(self);
	if (!mpd_qsetstatus(ctx, flags)) {
		INTERNAL_ERROR_INT("context_setstatus_dict"); /* GCOV_NOT_REACHED */
	}

	return 0;
}

static int
context_setclamp(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_context_t *ctx;
	mpd_ssize_t x;

	x = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return -1;
	}
	BOUNDS_CHECK(x, INT_MIN, INT_MAX);

	ctx = CtxAddr(self);
	if (!mpd_qsetclamp(ctx, (int)x)) {
		return value_error_int("valid values for clamp are 0 or 1.");
	}

	return 0;
}

static int
context_setallcr(PyObject *self, PyObject *value, void *closure UNUSED)
{
	mpd_context_t *ctx;
	mpd_ssize_t x;

	x = PyLong_AsMpdSsize(value);
	if (PyErr_Occurred()) {
		return -1;
	}
	BOUNDS_CHECK(x, INT_MIN, INT_MAX);

	ctx = CtxAddr(self);
	if (!mpd_qsetcr(ctx, (int)x)) {
		return value_error_int("valid values for _allcr are 0 or 1.");
	}

	return 0;
}

static PyObject *
context_getattr(PyObject *self, PyObject *name)
{
	PyObject *retval;

	if (!PyUnicode_Check(name)) {
		PyErr_Format(PyExc_TypeError, /* GCOV_NOT_REACHED (why?) */
		             "attribute name must be string, not '%.200s'",
		             name->ob_type->tp_name); /* GCOV_NOT_REACHED */
		return NULL; /* GCOV_NOT_REACHED */
	}

	if (PyUnicode_CompareWithASCIIString(name, "traps") == 0) {
		retval = ((PyDecContextObject *)self)->traps;
		Py_INCREF(retval);
		return retval;
	}
	else if (PyUnicode_CompareWithASCIIString(name, "flags") == 0) {
		retval = ((PyDecContextObject *)self)->flags;
		Py_INCREF(retval);
		return retval;
	}
	else {
		return PyObject_GenericGetAttr(self, name);
	}
}

static int
context_setattr(PyObject *self, PyObject *name, PyObject *value)
{
	if (!PyUnicode_Check(name)) {
		PyErr_Format(PyExc_TypeError, /* GCOV_NOT_REACHED (why?) */
		             "attribute name must be string, not '%.200s'",
		             name->ob_type->tp_name); /* GCOV_NOT_REACHED */
		return -1; /* GCOV_NOT_REACHED */
	}

	if (PyUnicode_CompareWithASCIIString(name, "traps") == 0) {
		if (value == NULL) {
			return value_error_int("traps cannot be deleted.");
		}
		return context_settraps_dict(self, value);
	}
	else if (PyUnicode_CompareWithASCIIString(name, "flags") == 0) {
		if (value == NULL) {
			return value_error_int("flags cannot be deleted.");
		}
		return context_setstatus_dict(self, value);
	}
	else {
		return PyObject_GenericSetAttr(self, name, value);
	}
}

static PyObject *
context_clear_traps(PyObject *self, PyObject *dummy UNUSED)
{
	PyDecContextObject *decctx = (PyDecContextObject *)self;

	if (signaldict_clear_all(decctx->traps) < 0) {
		return NULL; /* GCOV_UNLIKELY */
	}
	Py_RETURN_NONE;
}

static PyObject *
context_clear_flags(PyObject *self, PyObject *dummy UNUSED)
{
	PyDecContextObject *decctx = (PyDecContextObject *)self;

	if (signaldict_clear_all(decctx->flags) < 0) {
		return NULL; /* GCOV_UNLIKELY */
	}
	Py_RETURN_NONE;
}

static PyObject *
context_new(PyTypeObject *type UNUSED, PyObject *args UNUSED,
            PyObject *kwds UNUSED)
{
	PyDecContextObject *self = NULL;
	mpd_context_t *ctx;

	self = PyObject_New(PyDecContextObject, &PyDecContext_Type);
	if (self == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	self->traps = PyObject_CallObject((PyObject *)&PyDecSignalDict_Type, NULL);
	if (self->traps == NULL) {
		Py_DECREF(self); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	self->flags = PyObject_CallObject((PyObject *)&PyDecSignalDict_Type, NULL);
	if (self->flags == NULL) {
		Py_DECREF(self->traps); /* GCOV_UNLIKELY */
		Py_DECREF(self); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	ctx = CtxAddr(self);
	SdFlagAddr(self->traps) = &ctx->traps;
	SdFlagAddr(self->flags) = &ctx->status;

	return (PyObject *)self;
}

static void
context_dealloc(PyDecContextObject *self)
{
	Py_DECREF(self->traps);
	Py_DECREF(self->flags);
	PyObject_Del(self);
}

#ifdef CONFIG_64
  #define DEC_DFLT_EMAX 999999999
  #define DEC_DFLT_EMIN -999999999
#else
  #define DEC_DFLT_EMAX MPD_MAX_EMAX
  #define DEC_DFLT_EMIN MPD_MIN_EMIN
#endif

static mpd_context_t dflt_ctx = {
  28, DEC_DFLT_EMAX, DEC_DFLT_EMIN,
  MPD_IEEE_Invalid_operation|MPD_Division_by_zero|MPD_Overflow,
  0, 0, MPD_ROUND_HALF_EVEN, 0, 1
};

static int
context_init(PyObject *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {
	  "prec", "rounding", "Emin", "Emax", "capitals", "clamp",
	  "flags", "traps", "_allcr", NULL
	};
	PyObject *traps = NULL;
	PyObject *status = NULL;
	mpd_context_t *ctx, t=dflt_ctx;
	int capitals = 1;
	int ret;

	assert(PyTuple_Check(args));
	ctx = CtxAddr(self);

	if (default_context_template) {
		t = *CtxAddr(default_context_template);
	}
	if (!PyArg_ParseTupleAndKeywords(
	        args, kwds,
	        "|" CONV_mpd_ssize_t "i" CONV_mpd_ssize_t CONV_mpd_ssize_t "ii"
		"OOi", kwlist,
	        &t.prec, &t.round, &t.emin, &t.emax, &capitals, &t.clamp,
	        &status, &traps, &t.allcr
	     )) {
		return -1;
	}

	if (!mpd_qsetprec(ctx, t.prec) ||
	    !mpd_qsetemin(ctx, t.emin) ||
	    !mpd_qsetemax(ctx, t.emax) ||
	    !mpd_qsetround(ctx, t.round) ||
	    !mpd_qsettraps(ctx, t.traps) ||
	    !mpd_qsetstatus(ctx, t.status) ||
	    !mpd_qsetclamp(ctx, t.clamp) ||
	    !mpd_qsetcr(ctx, t.allcr)) {
		return value_error_int("invalid context.");
	}

	if (capitals != 0 && capitals != 1) {
		return value_error_int("invalid context.");
	}
	CtxCaps(self) = capitals;

	if (traps != NULL) {
		if (PyLong_Check(traps)) {
			ret = context_settraps(self, traps, NULL);
		}
		else if (PyList_Check(traps)) {
			ret = context_settraps_list(self, traps);
		}
		else {
			ret = context_settraps_dict(self, traps);
		}
		if (ret < 0) {
			return ret;
		}
	}
	if (status != NULL) {
		if (PyLong_Check(status)) {
			ret = context_setstatus(self, status, NULL);
		}
		else if (PyList_Check(status)) {
			ret = context_setstatus_list(self, status);
		}
		else {
			ret = context_setstatus_dict(self, status);
		}
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

#define FD_CTX_LEN 432
static PyObject *
context_repr(PyDecContextObject *self)
{
	mpd_context_t *ctx;
	char s[FD_CTX_LEN];
	char *cp;
	int n, mem;

	assert(PyDecContext_Check(self));
	ctx = CtxAddr(self);

	cp = s; mem = FD_CTX_LEN;
	n = snprintf(cp, mem,
	             "Context(prec=%"PRI_mpd_ssize_t", rounding=%s, "
	             "Emin=%"PRI_mpd_ssize_t", Emax=%"PRI_mpd_ssize_t", "
	             "capitals=%d, clamp=%d, flags=",
	             ctx->prec, mpd_round_string[ctx->round],
	             ctx->emin, ctx->emax,
	             self->capitals, ctx->clamp);
	if (n < 0 || n >= mem) goto error;
	cp += n; mem -= n;

	n = mpd_lsnprint_signals(cp, mem, ctx->status, dec_signal_string);
	if (n < 0 || n >= mem) goto error;
	cp += n; mem -= n;

	n = snprintf(cp, mem, ", traps=");
	if (n < 0 || n >= mem) goto error;
	cp += n; mem -= n;

	n = mpd_lsnprint_signals(cp, mem, ctx->traps, dec_signal_string);
	if (n < 0 || n >= mem) goto error;
	cp += n; mem -= n;

	n = snprintf(cp, mem, ")");
	if (n < 0 || n >= mem) goto error;

	return PyUnicode_FromString(s);

error: /* GCOV_NOT_REACHED */
	INTERNAL_ERROR_PTR("context_repr"); /* GCOV_NOT_REACHED */
}

static void
init_basic_context(PyObject *v)
{
	mpd_context_t ctx = dflt_ctx;

	ctx.prec = 9;
	ctx.traps |= (MPD_Underflow|MPD_Clamped);
	ctx.round = MPD_ROUND_HALF_UP;

	*CtxAddr(v) = ctx;
	CtxCaps(v) = 1;
}

static void
init_extended_context(PyObject *v)
{
	mpd_context_t ctx = dflt_ctx;

	ctx.prec = 9;
	ctx.traps = 0;

	*CtxAddr(v) = ctx;
	CtxCaps(v) = 1;
}

/* Factory function for creating IEEE interchange format contexts */
static PyObject *
ieee_context(PyObject *dummy UNUSED, PyObject *v)
{
	PyObject *ctxobj;
	mpd_ssize_t bits;
	mpd_context_t ctx;

	bits = PyLong_AsMpdSsize(v);
	if (PyErr_Occurred()) {
		return NULL;
	}
	if (bits <= 0 || bits > INT_MAX) {
		goto error;
	}
	if (mpd_ieee_context(&ctx, (int)bits) < 0) {
		goto error;
	}

	ctxobj = PyObject_CallObject((PyObject *)&PyDecContext_Type, NULL);
	if (ctxobj == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	*CtxAddr(ctxobj) = ctx;

	return ctxobj;

error:
	PyErr_Format(PyExc_ValueError,
	    "argument must be a multiple of 32, with a maximum of %d.",
	    MPD_IEEE_CONTEXT_MAX_BITS);

	return NULL;
}

static PyObject *
context_copy(PyObject *self)
{
	PyObject *newob;
	mpd_context_t *ctx;

	newob = PyObject_CallObject((PyObject *)&PyDecContext_Type, NULL);
	if (newob == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	ctx = CtxAddr(newob);

	*ctx = *CtxAddr(self);
	ctx->newtrap = 0;
	CtxCaps(newob) = CtxCaps(self);

	return newob;
}

static PyObject *
context_reduce(PyObject *self, PyObject *args UNUSED)
{
	mpd_context_t *ctx = CtxAddr(self);

	return Py_BuildValue(
	        "O(" CONV_mpd_ssize_t "i" CONV_mpd_ssize_t CONV_mpd_ssize_t
	        "iiiii)",
	        Py_TYPE(self), ctx->prec, ctx->round, ctx->emin, ctx->emax,
	        CtxCaps(self), ctx->clamp, ctx->status, ctx->traps, ctx->allcr
	);
}

static PyObject *
PyDec_SetStatusFromList(PyObject *self, PyObject *value)
{
	if (context_setstatus_list(self, value) < 0) {
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject *
PyDec_SetTrapsFromList(PyObject *self, PyObject *value)
{
	if (context_settraps_list(self, value) < 0) {
		return NULL;
	}
	Py_RETURN_NONE;
}


static PyGetSetDef context_getsets [] =
{
  { "prec", (getter)context_getprec, (setter)context_setprec, NULL, NULL},
  { "Emax", (getter)context_getemax, (setter)context_setemax, NULL, NULL},
  { "Emin", (getter)context_getemin, (setter)context_setemin, NULL, NULL},
  { "rounding", (getter)context_getround, (setter)context_setround, NULL, NULL},
  { "capitals", (getter)context_getcapitals, (setter)context_setcapitals, NULL, NULL},
  { "clamp", (getter)context_getclamp, (setter)context_setclamp, NULL, NULL},
  { "_clamp", (getter)context_getclamp, (setter)context_setclamp, NULL, NULL},
  { "_allcr", (getter)context_getallcr, (setter)context_setallcr, NULL, NULL},
  { "_traps", (getter)context_gettraps, (setter)context_settraps, NULL, NULL},
  { "_flags", (getter)context_getstatus, (setter)context_setstatus, NULL, NULL},
  {NULL}
};


#define CONTEXT_CHECK(obj) \
        if (!PyDecContext_Check(obj)) {             \
                PyErr_SetString(PyExc_TypeError,    \
                    "argument must be a context."); \
                return NULL;                        \
        }

#define CONTEXT_CHECK_VA(obj) \
        if (!PyDecContext_Check(obj)) {                      \
                PyErr_SetString(PyExc_TypeError,             \
                    "optional argument must be a context."); \
                return NULL;                                 \
        }


/******************************************************************************/
/*                Global, thread local and temporary contexts                 */
/******************************************************************************/

#ifdef WITHOUT_THREADS
/* Return borrowed reference to the current context. When compiled
 * without threads, this is always the module context. */
static int module_context_set = 0;
static PyObject *
current_context(void)
{
	/* In decimal.py, the module context is automatically initialized
	 * from the DefaultContext when it is first accessed. This
	 * complicates the code and has a speed penalty of 1-2%. */
	if (module_context_set) {
		return module_context;
	}

	*CtxAddr(module_context) = *CtxAddr(default_context_template);
	module_context_set = 1;
	return module_context;
}

/* ctxobj := borrowed reference to the current context */
#define CURRENT_CONTEXT(ctxobj) \
        ctxobj = current_context()

/* ctx := pointer to the mpd_context_t struct of the current context */
#define CURRENT_CONTEXT_ADDR(ctx) \
        ctx = CtxAddr(current_context())

/* Return current context, increment reference */
static PyObject *
PyDec_GetCurrentContext(void)
{
	PyObject *ctxobj;

	CURRENT_CONTEXT(ctxobj);

	Py_INCREF(ctxobj);
	return ctxobj;
}

/* Set the module context to a new context, decrement old reference */
static PyObject *
PyDec_SetCurrentContext(PyObject *self UNUSED, PyObject *v)
{
	CONTEXT_CHECK(v);

	/* If the new context is one of the templates, make a copy.
	 * This is the current behavior of decimal.py. */
	if (v == default_context_template ||
	    v == basic_context_template ||
	    v == extended_context_template) {
		if ((v = context_copy(v)) == NULL) {
			return NULL;
		}
	}
	else {
		Py_INCREF(v);
	}

	Py_XDECREF(module_context);
	module_context = v;
	module_context_set = 1;
	Py_RETURN_NONE;
}
#else
/*
 * Thread local storage currently has a speed penalty of about 16%.
 * All functions that map Python's arithmetic operators to mpdecimal
 * functions have to look up the current context for each and every
 * operation.
 */

/* Return borrowed reference to thread local context. */
static PyObject *
current_context(void)
{
	PyObject *dict = NULL;
	PyObject *tl_context = NULL;

	dict = PyThreadState_GetDict();
	if (dict == NULL) {
		PyErr_SetString(PyExc_RuntimeError, /* GCOV_UNLIKELY */
		    "cannot get thread state."); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	tl_context = PyDict_GetItem(dict, tls_context_key);
	if (tl_context != NULL) {
		/* We already have a thread local context and
		 * return a borrowed reference. */
		CONTEXT_CHECK(tl_context);
		return tl_context;
	}

	/* Otherwise, set up a new thread local context. */
	tl_context = (PyObject *)context_copy(default_context_template);
	if (tl_context == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	if (PyDict_SetItem(dict, tls_context_key, tl_context) < 0) {
		Py_DECREF(tl_context); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	Py_DECREF(tl_context);

	/* refcount is 1 */
	return tl_context;
}

/* ctxobj := borrowed reference to the current context */
#define CURRENT_CONTEXT(ctxobj) \
        if ((ctxobj = current_context()) == NULL) { \
                return NULL;                        \
        }

/* ctx := pointer to the mpd_context_t struct of the current context */
#define CURRENT_CONTEXT_ADDR(ctx) { \
        PyObject *_c_t_x_o_b_j = current_context(); \
        if (_c_t_x_o_b_j == NULL) {                 \
                return NULL;                        \
        }                                           \
        ctx = CtxAddr(_c_t_x_o_b_j);                \
}

/* Return current context, increment reference */
static PyObject *
PyDec_GetCurrentContext(void)
{
	PyObject *obj;

	if ((obj = current_context()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	Py_INCREF(obj);
	return obj;
}

/* Set the thread local context to a new context, decrement old reference */
static PyObject *
PyDec_SetCurrentContext(PyObject *self UNUSED, PyObject *v)
{
	PyObject *dict;

	CONTEXT_CHECK(v);

	dict = PyThreadState_GetDict();
	if (dict == NULL) {
		PyErr_SetString(PyExc_RuntimeError, /* GCOV_UNLIKELY */
		    "cannot get thread state."); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	/* If the new context is one of the templates, make a copy.
	 * This is the current behavior of decimal.py. */
	if (v == default_context_template ||
	    v == basic_context_template ||
	    v == extended_context_template) {
		if ((v = context_copy(v)) == NULL) {
			return NULL; /* GCOV_UNLIKELY */
		}
	}
	else {
		Py_INCREF(v);
	}

	if (PyDict_SetItem(dict, tls_context_key, v) < 0) {
		return NULL; /* GCOV_UNLIKELY */
	}

	Py_DECREF(v);
	Py_RETURN_NONE;
}
#endif

/* Context manager object for the 'with' statement. The manager
 * owns one reference to the global (outer) context and one
 * to the local (inner) context. */
static PyObject *
ctxmanager_new(PyTypeObject *type UNUSED, PyObject *args)
{
	PyDecContextManagerObject *self;
	PyObject *local;
	PyObject *global;

	CURRENT_CONTEXT(global);
	local = global;
	if (!PyArg_ParseTuple(args, "|O", &local)) {
		return NULL; /* GCOV_NOT_REACHED */
	}
	CONTEXT_CHECK_VA(local);

	self = PyObject_New(PyDecContextManagerObject,
	                    &PyDecContextManager_Type);
	if (self == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	self->local = context_copy(local);
	if (self->local == NULL) {
		Py_DECREF(self); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	self->global = global;
	Py_INCREF(self->global);

	return (PyObject *)self;
}

static void
ctxmanager_dealloc(PyDecContextManagerObject *self)
{
	Py_DECREF(self->local);
	Py_DECREF(self->global);
	PyObject_Del(self);
}

static PyObject *
ctxmanager_set_local(PyDecContextManagerObject *self, PyObject *args UNUSED)
{
	PyObject *ret;

	ret = PyDec_SetCurrentContext(NULL, self->local);
	if (ret == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	Py_DECREF(ret);

	Py_INCREF(self->local);
	return self->local;
}

static PyObject *
ctxmanager_restore_global(PyDecContextManagerObject *self,
                           PyObject *args UNUSED)
{
	PyObject *ret;

	ret = PyDec_SetCurrentContext(NULL, self->global);
	if (ret == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	Py_DECREF(ret);

	Py_RETURN_NONE;
}


static PyMethodDef ctxmanager_methods[] = {
  {"__enter__", (PyCFunction)ctxmanager_set_local, METH_NOARGS, NULL},
  {"__exit__", (PyCFunction)ctxmanager_restore_global, METH_VARARGS, NULL},
  {NULL, NULL}
};

static PyTypeObject PyDecContextManager_Type =
{
	PyVarObject_HEAD_INIT(NULL, 0)
	"cdecimal.ContextManager",              /* tp_name */
	sizeof(PyDecContextManagerObject),      /* tp_basicsize */
	0,                                      /* tp_itemsize */
	(destructor) ctxmanager_dealloc,        /* tp_dealloc */
	0,                                      /* tp_print */
	(getattrfunc) 0,                        /* tp_getattr */
	(setattrfunc) 0,                        /* tp_setattr */
	0,                                      /* tp_compare */
	(reprfunc) 0,                           /* tp_repr */
	0,                                      /* tp_as_number */
	0,                                      /* tp_as_sequence */
	0,                                      /* tp_as_mapping */
	0,                                      /* tp_hash */
	0,                                      /* tp_call */
	0,                                      /* tp_str */
	(getattrofunc) PyObject_GenericGetAttr, /* tp_getattro */
	(setattrofunc) 0,                       /* tp_setattro */
	(PyBufferProcs *) 0,                    /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                     /* tp_flags */
	0,                                      /* tp_doc */
	0,                                      /* tp_traverse */
	0,                                      /* tp_clear */
	0,                                      /* tp_richcompare */
	0,                                      /* tp_weaklistoffset */
	0,                                      /* tp_iter */
	0,                                      /* tp_iternext */
	ctxmanager_methods,                     /* tp_methods */
};


/******************************************************************************/
/*                             Decimal Object                                 */
/******************************************************************************/

static PyDecObject *
dec_alloc(void)
{
	PyDecObject * self;

	if ((self = PyObject_New(PyDecObject, &PyDec_Type)) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((self->dec = mpd_qnew()) == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		PyObject_Del(self); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return self;
}

static void
dec_dealloc(PyObject *self)
{
	mpd_del(DecAddr(self));
	Py_TYPE(self)->tp_free(self);
}


/*******************************************************/
/*               Conversions to decimal                */
/*******************************************************/

/* Caller guarantees types. */
static PyObject *
_PyDec_FromUnicode(PyObject *v, mpd_context_t *ctx)
{
	PyDecObject *newob;
	uint32_t status = 0;
	char *cp;

	if((newob = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((cp = PyMem_Malloc(PyUnicode_GET_SIZE(v)+1)) == NULL) {
		Py_DECREF(newob); /* GCOV_UNLIKELY */
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	if (PyUnicode_EncodeDecimal(PyUnicode_AS_UNICODE(v),
	                            PyUnicode_GET_SIZE(v),
	                            cp, NULL)) {
		Py_DECREF(newob); /* GCOV_UNLIKELY */
		PyMem_Free(cp); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qset_string(newob->dec, cp, ctx, &status);
	PyMem_Free(cp);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(newob);
		return NULL;
	}

	return (PyObject *)newob;
}

/* Caller guarantees types. */
static PyObject *
_PyDec_FromLong(PyObject *v, mpd_context_t *ctx)
{
	PyDecObject *newob;
	PyLongObject *l = (PyLongObject *)v;
	Py_ssize_t ob_size;
	uint32_t status = 0;
	size_t len;
	uint8_t sign;

	if((newob = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	ob_size = Py_SIZE(l);
	if (ob_size == 0) {
		newob->dec->exp = 0;
		newob->dec->data[0] = 0;
		newob->dec->len = 1;
		newob->dec->digits = 1;
		return (PyObject *)newob;
	}

	if (ob_size < 0) {
		len = -ob_size;
		sign = MPD_NEG;
	}
	else {
		len = ob_size;
		sign = MPD_POS;
	}

#if PYLONG_BITS_IN_DIGIT == 30
	mpd_qimport_u32(newob->dec, l->ob_digit, len, sign, PyLong_BASE,
	                ctx, &status);
#elif PYLONG_BITS_IN_DIGIT == 15
	mpd_qimport_u16(newob->dec, l->ob_digit, len, sign, PyLong_BASE,
	                ctx, &status);
#else
  #error "PYLONG_BITS_IN_DIGIT should be 15 or 30."
#endif
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(newob);
		return NULL;
	}

	return (PyObject *)newob;
}

/*
 * The following conversion functions read with a maxcontext in order
 * to emulate limitless reading of decimals. If the decimal cannot be
 * read exactly due to the limits of the maxcontext, InvalidOperation
 * is raised.
 *
 * Leading and trailing whitespace is allowed.
 */
static char *
strip_ws(const char *x)
{
	char *s, *t;
	char *y;
	size_t n;

	s = (char *)x;
	while (isspace((unsigned char)*s))
		s++;

	t = y = s+strlen(s);
	while (t > s && isspace((unsigned char)*(t-1)))
		t--;

	if (s != x || t != y) {
		n = t-s;
		if ((y = PyMem_Malloc(n+1)) == NULL) {
			return NULL; /* GCOV_UNLIKELY */
		}
		strncpy(y, s, n);
		y[n] = '\0';
		return y;
	}

	return (char *)x;
}

/* Caller guarantees types. */
static PyObject *
_PyDec_FromUnicode_Max(PyObject *v, mpd_context_t *ctx)
{
	PyDecObject *newob;
	mpd_context_t maxctx;
	uint32_t status = 0;
	char *cp, *stripped;

	if((newob = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((cp = PyMem_Malloc(PyUnicode_GET_SIZE(v)+1)) == NULL) {
		Py_DECREF(newob); /* GCOV_UNLIKELY */
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	if (PyUnicode_EncodeDecimal(PyUnicode_AS_UNICODE(v),
	                            PyUnicode_GET_SIZE(v),
	                            cp, NULL)) {
		Py_DECREF(newob); /* GCOV_UNLIKELY */
		PyMem_Free(cp); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_maxcontext(&maxctx);
	if ((stripped = strip_ws(cp)) == NULL) {
		Py_DECREF(newob); /* GCOV_UNLIKELY */
		PyMem_Free(cp); /* GCOV_UNLIKELY */
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qset_string(newob->dec, stripped, &maxctx, &status);
	if (stripped != cp) {
		PyMem_Free(stripped);
	}
	PyMem_Free(cp);
	if (status&(MPD_Inexact|MPD_Rounded)) {
		/* we want exact results */
		mpd_seterror(newob->dec, MPD_Invalid_operation, &status);
	}
	status &= MPD_Errors;
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(newob); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *)newob;
}

/* Caller guarantees types. */
static PyObject *
_PyDec_FromLong_Max(PyObject *v, mpd_context_t *ctx)
{
	PyDecObject *newob;
	PyLongObject *l = (PyLongObject *)v;
	Py_ssize_t ob_size;
	mpd_context_t maxctx;
	uint32_t status = 0;
	size_t len;
	uint8_t sign;

	if((newob = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	ob_size = Py_SIZE(l);
	if (ob_size == 0) {
		newob->dec->exp = 0;
		newob->dec->data[0] = 0;
		newob->dec->len = 1;
		newob->dec->digits = 1;
		return (PyObject *)newob;
	}

	if (ob_size < 0) {
		len = -ob_size;
		sign = MPD_NEG;
	}
	else {
		len = ob_size;
		sign = MPD_POS;
	}

	mpd_maxcontext(&maxctx);

#if PYLONG_BITS_IN_DIGIT == 30
	mpd_qimport_u32(newob->dec, l->ob_digit, len, sign, PyLong_BASE,
	                &maxctx, &status);
#elif PYLONG_BITS_IN_DIGIT == 15
	mpd_qimport_u16(newob->dec, l->ob_digit, len, sign, PyLong_BASE,
	                &maxctx, &status);
#else
  #error "PYLONG_BITS_IN_DIGIT should be 15 or 30."
#endif

	if (status&(MPD_Inexact|MPD_Rounded)) {
		/* we want exact results */
		mpd_seterror(newob->dec, MPD_Invalid_operation, &status); /* GCOV_UNLIKELY */
	}
	status &= MPD_Errors;
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(newob); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *)newob;
}

/* Caller guarantees types. */
static PyObject *
string_from_tuple(PyObject *v)
{
	PyObject *tmp, *dtuple;
	char *decstring, *cp;
	char sign_special[6];
	long sign, l;
	mpd_ssize_t exp = 0;
	Py_ssize_t i, mem, tsize;
	int n;

	if (PyTuple_Size(v) != 3) {
		return value_error_ptr(
		    "argument must be a tuple of length 3.");
	}

	tmp = PyTuple_GET_ITEM(v, 0);
	if (!PyLong_Check(tmp)) {
		return value_error_ptr(
		    "sign must be an integer with the value 0 or 1.");
	}
	sign = PyLong_AsLong(tmp);
	if (sign != 0 && sign != 1) {
		return value_error_ptr(
		    "sign must be an integer with the value 0 or 1.");
	}
	sign_special[0] = sign ? '-' : '+';
	sign_special[1] = '\0';

	tmp = PyTuple_GET_ITEM(v, 2);
	if (PyUnicode_Check(tmp)) {
		if (PyUnicode_CompareWithASCIIString(tmp, "F") == 0) {
			strcat(sign_special, "Inf");
		}
		else if (PyUnicode_CompareWithASCIIString(tmp, "n") == 0) {
			strcat(sign_special, "NaN");
		}
		else if (PyUnicode_CompareWithASCIIString(tmp, "N") == 0) {
			strcat(sign_special, "sNaN");
		}
		else {
			return value_error_ptr(
			    "string argument in the third position "
			    "must be 'F', 'n' or 'N'.");
		}
	}
	else {
		if (!PyLong_Check(tmp)) {
			return value_error_ptr(
			    "exponent must be an integer.");
		}
		exp = PyLong_AsMpdSsize(tmp);
		if (PyErr_Occurred()) {
			return NULL; /* GCOV_UNLIKELY */
		}
	}

	dtuple = PyTuple_GET_ITEM(v, 1);
	if (!PyTuple_Check(dtuple)) {
		return value_error_ptr(
		    "coefficient must be a tuple of digits.");
	}

	tsize = PyTuple_Size(dtuple);
	/* [sign][tuple-digits+1][E][-][exp-digits+1]['\0'] */
	mem = 1 + tsize + 3 + MPD_EXPDIGITS + 2;
	cp = decstring = PyMem_Malloc(mem);
	if (decstring == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	n = snprintf(cp, mem, "%s", sign_special);
	if (n < 0 || n >= mem) return NULL;
	cp += n;

	if (sign_special[1] == '\0' && tsize == 0) {
		/* not a special number and empty tuple */
		*cp++ = '0';
	}
	for (i = 0; i < tsize; i++) {
		tmp = PyTuple_GET_ITEM(dtuple, i);
		if (!PyLong_Check(tmp)) {
			PyMem_Free(decstring);
			return value_error_ptr(
			    "coefficient must be a tuple of digits.");
		}
		l = PyLong_AsLong(tmp);
		if (l < 0 || l > 9) {
			PyMem_Free(decstring);
			return value_error_ptr(
			    "coefficient must be a tuple of digits.");
		}
		*cp++ = (char)l + '0';
	}
	*cp = '\0';

	if (sign_special[1] == '\0') {
		/* not a special number */
		*cp++ = 'E';
		n = snprintf(cp, MPD_EXPDIGITS+1, "%" PRI_mpd_ssize_t, exp);
		if (n < 0 || n >= MPD_EXPDIGITS+1) return NULL;
	}

	tmp = PyUnicode_FromString(decstring);
	PyMem_Free(decstring);

	return tmp;
}

static PyObject *
_PyDec_FromTuple_Max(PyObject *v, mpd_context_t *ctx)
{
	PyObject *result;
	PyObject *s;

	s = string_from_tuple(v);
	if (s == NULL) {
		return NULL;
	}
	result = _PyDec_FromUnicode_Max(s, ctx);
	Py_DECREF(s);
	return result;
}

static PyObject *
_PyDec_FromTuple(PyObject *v, mpd_context_t *ctx)
{
	PyObject *result;
	PyObject *s;

	s = string_from_tuple(v);
	if (s == NULL) {
		return NULL;
	}
	result = _PyDec_FromUnicode(s, ctx);
	Py_DECREF(s);
	return result;
}

static PyObject *
_PyDec_FromFloat_Max(PyObject *self, PyObject *v)
{
	PyObject *result, *tmp;
	PyObject *n, *d, *n_d;
	mpd_ssize_t k;
	double x;
	int sign;
	mpd_t *d1, *d2;
	uint32_t status = 0;
	mpd_context_t *ctx, maxctx;


	CURRENT_CONTEXT_ADDR(ctx);
	if (PyLong_Check(v)) {
		return _PyDec_FromLong_Max(v, ctx);
	}

	x = PyFloat_AsDouble(v);
	if (x == -1.0 && PyErr_Occurred()) {
		return NULL;
	}
	sign = (copysign(1.0, x) == 1.0) ? 0 : 1;

	if (Py_IS_NAN(x) || Py_IS_INFINITY(x)) {
		result = PyObject_CallObject(self, NULL);
		if (result == NULL) {
			return NULL; /* GCOV_UNLIKELY */
		}
		if (Py_IS_NAN(x)) {
			/* decimal.py calls repr(float(+-nan)),
			 * which always gives a positive result. */
			mpd_setspecial(DecAddr(result), MPD_POS, MPD_NAN);
		}
		else {
			mpd_setspecial(DecAddr(result), sign, MPD_INF);
		}
		return result;
	}

	if ((tmp = PyObject_CallMethod(v, "__abs__", NULL)) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	n_d = PyObject_CallMethod(tmp, "as_integer_ratio", NULL);
	Py_DECREF(tmp);
	if (n_d == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((n = PyTuple_GetItem(n_d, 0)) == NULL) {
		Py_DECREF(n_d); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((d = PyTuple_GetItem(n_d, 1)) == NULL) {
		Py_DECREF(n_d); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}


	if ((tmp = PyObject_CallMethod(d, "bit_length", NULL)) == NULL) {
		Py_DECREF(n_d); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	k = PyLong_AsMpdSsize(tmp);
	Py_DECREF(tmp);
	if (k == MPD_SSIZE_MAX) {
		Py_DECREF(n_d); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	k--;

	if ((d1 = mpd_qnew()) == NULL) {
		Py_DECREF(n_d); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((d2 = mpd_qnew()) == NULL) {
		mpd_del(d1); /* GCOV_UNLIKELY */
		Py_DECREF(n_d); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_maxcontext(&maxctx);
	mpd_qset_uint(d1, 5, &maxctx, &status);
	mpd_qset_ssize(d2, k, &maxctx, &status);
	mpd_qpow(d1, d1, d2, &maxctx, &status);
	if (dec_addstatus(ctx, status)) {
		mpd_del(d1); /* GCOV_UNLIKELY */
		mpd_del(d2); /* GCOV_UNLIKELY */
		Py_DECREF(n_d); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	tmp = Py_BuildValue("(O)", n);
	result = PyObject_CallObject(self, tmp);
	Py_DECREF(tmp);
	Py_DECREF(n_d);
	if (result == NULL) {
		mpd_del(d1); /* GCOV_UNLIKELY */
		mpd_del(d2); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	/* result = n * 5**k */
	mpd_qmul(DecAddr(result), DecAddr(result), d1, &maxctx, &status);
	mpd_del(d1);
	mpd_del(d2);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	/* result = +- n * 5**k * 10**-k */
	mpd_set_sign(DecAddr(result), sign);
	DecAddr(result)->exp = -k;

	return result;
}

static PyObject *
PyDecContext_FromFloat(PyObject *self, PyObject *v)
{
	PyObject *result;
	mpd_context_t *ctx;
	uint32_t status = 0;

	ctx = CtxAddr(self);

	result = _PyDec_FromFloat_Max((PyObject *)&PyDec_Type, v);
	if (result == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qfinalize(DecAddr(result), ctx, &status);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result);
		return NULL;
	}

	return result;
}

/* Caller guarantees types. */
static PyObject *
dec_apply(PyObject *v, mpd_context_t *ctx)
{
	PyDecObject *newob;
	uint32_t status = 0;

	if((newob = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qcopy(newob->dec, ((PyDecObject *)v)->dec, &status);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(newob); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qfinalize(newob->dec, ctx, &status);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(newob);
		return NULL;
	}

	return (PyObject *)newob;
}

static PyObject *
PyDec_Apply(PyObject *decobj, PyObject *args)
{
	PyObject *ctxobj;
	mpd_context_t *ctx;

	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTuple(args, "|O", &ctxobj)) {
		return NULL;
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	return dec_apply(decobj, ctx);
}

static PyObject *
PyDecContext_Apply(PyObject *ctxobj, PyObject *decobj)
{
	mpd_context_t *ctx;

	if (!PyDec_Check(decobj)) {
		PyErr_SetString(PyExc_TypeError,
		    "argument must be a Decimal.");
		return NULL;
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	return dec_apply(decobj, ctx);
}


/* Conversion functions */

/* Try to convert PyObject v to PyDecObject a. */
static inline int
convert_op(PyObject *v, PyDecObject **a, mpd_context_t *ctx)
{

	if (PyDec_Check(v)) {
		*a = (PyDecObject *) v;
		Py_INCREF(v);
		return 1;
	}
	if (PyLong_Check(v)) {
		*a = (PyDecObject *) _PyDec_FromLong_Max(v, ctx);
		if (*a == NULL) {
			return 0; /* GCOV_UNLIKELY */
		}
		return 1;
	}

	Py_INCREF(Py_NotImplemented);
	*a = (PyDecObject *) Py_NotImplemented;
	return 0;
}

#define CONVERT_OP(v, a, ctx) \
        if (!convert_op(v, a, ctx)) {     \
                return (PyObject *) *(a); \
        }

#define CONVERT_BINOP(v, w, a, b, ctx) \
        if (!convert_op(v, a, ctx)) {     \
                return (PyObject *) *(a); \
        }                                 \
        if (!convert_op(w, b, ctx)) {     \
                Py_DECREF(*(a));          \
                return (PyObject *) *(b); \
        }

#define CONVERT_TERNOP(v, w, x, a, b, c, ctx) \
        if (!convert_op(v, a, ctx)) {         \
                return (PyObject *) *(a);     \
        }                                     \
        if (!convert_op(w, b, ctx)) {         \
                Py_DECREF(*(a));              \
                return (PyObject *) *(b);     \
        }                                     \
        if (!convert_op(x, c, ctx)) {         \
                Py_DECREF(*(a));              \
                Py_DECREF(*(b));              \
                return (PyObject *) *(c);     \
        }


/* Convert for comparison */
static int
convert_op_cmp(PyObject *v, PyDecObject **a, int op, mpd_context_t *ctx)
{
	if (PyDec_Check(v)) {
		*a = (PyDecObject *) v;
		Py_INCREF(v);
		return 1;
	}
	if (PyLong_Check(v)) {
		*a = (PyDecObject *) _PyDec_FromLong_Max(v, ctx);
		if (*a == NULL) {
			return 0; /* GCOV_UNLIKELY */
		}
		return 1;
	}
	if (PyFloat_Check(v)) {
		ctx->status |= MPD_Float_operation;
		if (op != Py_EQ && op != Py_NE &&
		    dec_addstatus(ctx, MPD_Float_operation)) {
			*a = NULL;
			return 0;
		}
#if PY_VERSION_HEX >= 0x03020000
		*a = (PyDecObject *) _PyDec_FromFloat_Max(
		                         (PyObject *)&PyDec_Type, v);
		if (*a == NULL) {
			return 0; /* GCOV_UNLIKELY */
		}
		return 1;
#endif
	}

	Py_INCREF(Py_NotImplemented);
	*a = (PyDecObject *) Py_NotImplemented;
	return 0;
}

#define CONVERT_BINOP_CMP(v, w, a, b, op, ctx) \
        if (!convert_op_cmp(v, a, op, ctx)) {  \
                return (PyObject *) *(a);      \
        }                                      \
        if (!convert_op_cmp(w, b, op, ctx)) {  \
                Py_DECREF(*(a));               \
                return (PyObject *) *(b);      \
        }


/* Same as convert_op(), but set an error instead of returning
 * NotImplemented. */
static int
convert_op_set(PyObject *v, PyDecObject **a, mpd_context_t *ctx)
{

	if (PyDec_Check(v)) {
		*a = (PyDecObject *) v;
		Py_INCREF(v);
		return 1;
	}
	if (PyLong_Check(v)) {
		*a = (PyDecObject *) _PyDec_FromLong_Max(v, ctx);
		if (*a == NULL) {
			return 0; /* GCOV_UNLIKELY */
		}
		return 1;
	}

	PyErr_Format(PyExc_TypeError,
	    "conversion from %s to Decimal is not supported.",
	    v->ob_type->tp_name);
	return 0;
}

#define CONVERT_OP_SET(v, a, ctx) \
        if (!convert_op_set(v, a, ctx)) { \
                return NULL;              \
        }

#define CONVERT_BINOP_SET(v, w, a, b, ctx) \
        if (!convert_op_set(v, a, ctx)) {  \
                return NULL;               \
        }                                  \
        if (!convert_op_set(w, b, ctx)) {  \
                Py_DECREF(*(a));           \
                return NULL;               \
        }

#define CONVERT_TERNOP_SET(v, w, x, a, b, c, ctx) \
        if (!convert_op_set(v, a, ctx)) {         \
                return NULL;                      \
        }                                         \
        if (!convert_op_set(w, b, ctx)) {         \
                Py_DECREF(*(a));                  \
                return NULL;                      \
        }                                         \
        if (!convert_op_set(x, c, ctx)) {         \
                Py_DECREF(*(a));                  \
                Py_DECREF(*(b));                  \
                return NULL;                      \
        }


static PyObject *dec_subtype_new(PyTypeObject *type, PyObject *args,
                                 PyObject *kwds);

static PyObject *
dec_new(PyTypeObject *type, PyObject *args, PyObject *kwds UNUSED)
{
	PyObject *v = NULL;
	PyObject *ctxobj;
	mpd_context_t *ctx;

	if (type != &PyDec_Type) {
		return dec_subtype_new(type, args, kwds);
	}

	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTuple(args, "|OO", &v, &ctxobj)) {
		return NULL;
	}

	if (v == NULL) {
		v = PyLong_FromLong(0);
		Py_DECREF(v);
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	if (PyDec_Check(v)) {
		Py_INCREF(v);
		return v;
	}
	else if (PyUnicode_Check(v)) {
		return _PyDec_FromUnicode_Max(v, ctx);
	}
	else if (PyLong_Check(v)) {
		return _PyDec_FromLong_Max(v, ctx);
	}
	else if (PyTuple_Check(v)) {
		return _PyDec_FromTuple_Max(v, ctx);
	}
#if PY_VERSION_HEX >= 0x03020000
	else if (PyFloat_Check(v)) {
		if (dec_addstatus(ctx, MPD_Float_operation)) {
			return NULL;
		}
		return _PyDec_FromFloat_Max((PyObject *)&PyDec_Type, v);
	}
#endif
	else {
		PyErr_Format(PyExc_TypeError,
		    "conversion from %s to Decimal is not supported.",
		    v->ob_type->tp_name);
		return NULL;
	}
}

static PyObject *
dec_subtype_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	PyObject *tmp, *newobj;

	assert(PyType_IsSubtype(type, &PyDec_Type));
	tmp = dec_new(&PyDec_Type, args, kwds);
	if (tmp == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	newobj = type->tp_alloc(type, 0);
	if (newobj == NULL) {
		Py_DECREF(tmp); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	DecAddr(newobj) = mpd_qncopy(DecAddr(tmp));
	if (DecAddr(newobj) == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		Py_DECREF(tmp); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	Py_DECREF(tmp);
	return (PyObject *)newobj;
}

static PyObject *
PyDecContext_CreateDecimal(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;
	mpd_context_t *ctx;

	if (!PyArg_ParseTuple(args, "|O", &v)) {
		return NULL;
	}

	if (v == NULL) {
		v = PyLong_FromLong(0);
		Py_DECREF(v);
	}

	ctx = CtxAddr(self);

	if (PyDec_Check(v)) {
		return dec_apply(v, ctx);
	}
	else if (PyUnicode_Check(v)) {
		return _PyDec_FromUnicode(v, ctx);
	}
	else if (PyLong_Check(v)) {
		return _PyDec_FromLong(v, ctx);
	}
	else if (PyTuple_Check(v)) {
		return _PyDec_FromTuple(v, ctx);
	}
#if PY_VERSION_HEX >= 0x03020000
	else if (PyFloat_Check(v)) {
		if (dec_addstatus(ctx, MPD_Float_operation)) {
			return NULL;
		}
		return PyDecContext_FromFloat(self, v);
	}
#endif
	else {
		PyErr_Format(PyExc_TypeError,
		    "conversion from %s to Decimal is not supported.",
		    v->ob_type->tp_name);
		return NULL;
	}
}


/*******************************************************/
/*              Conversions from decimal               */
/*******************************************************/

/* Caller guarantees type. Uses default module context. */
static PyObject *
_PyInt_FromDec(PyDecObject *self, mpd_context_t *ctx, int round)
{
	PyLongObject *newob;
	mpd_t *intdec;
	size_t maxsize, n;
	Py_ssize_t i;
	mpd_context_t workctx;
	uint32_t status = 0;

	if (mpd_isspecial(self->dec)) {
		if (mpd_isnan(self->dec)) {
			PyErr_SetString(PyExc_ValueError,
			    "cannot convert NaN to integer.");
		}
		else {
			PyErr_SetString(PyExc_OverflowError,
			    "cannot convert Infinity to integer.");
		}
		return NULL;
	}

	if ((intdec = mpd_qnew()) == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	workctx = *ctx;
	workctx.round = round;
	mpd_qround_to_int(intdec, self->dec, &workctx, &status);
	if (dec_addstatus(ctx, status)) {
		mpd_del(intdec); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	maxsize = mpd_sizeinbase(intdec, PyLong_BASE);
	if (maxsize > PY_SSIZE_T_MAX) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		mpd_del(intdec); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((newob = _PyLong_New(maxsize)) == NULL) {
		mpd_del(intdec); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	status = 0;
#if PYLONG_BITS_IN_DIGIT == 30
	n = mpd_qexport_u32(newob->ob_digit, maxsize, PyLong_BASE,
	                    intdec, &status);
#elif PYLONG_BITS_IN_DIGIT == 15
	n = mpd_qexport_u16(newob->ob_digit, maxsize, PyLong_BASE,
	                    intdec, &status);
#else
  #error "PYLONG_BITS_IN_DIGIT should be 15 or 30."
#endif
	if (dec_addstatus(ctx, status)) {
		Py_DECREF((PyObject *) newob); /* GCOV_UNLIKELY */
		mpd_del(intdec); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	i = n;
	while ((i > 0) && (newob->ob_digit[i-1] == 0)) {
		i--;
	}

	Py_SIZE(newob) = i;
	if (mpd_isnegative(intdec) && !mpd_iszero(intdec)) {
		Py_SIZE(newob) = -i;
	}

	mpd_del(intdec);
	return (PyObject *) newob;
}

static PyObject *
PyLong_FromDec(PyDecObject *self)
{
	mpd_context_t *ctx;

	CURRENT_CONTEXT_ADDR(ctx);
	return _PyInt_FromDec(self, ctx, MPD_ROUND_DOWN);
}

#if PY_VERSION_HEX < 0x03020000
/* Caller guarantees type */
static PyObject *
PyLong_FromDecCtx(PyDecObject *self, mpd_context_t *ctx)
{
	return _PyInt_FromDec(self, ctx, MPD_ROUND_DOWN);
}
#endif

/* Caller guarantees type. Uses default module context. */
static PyObject *
PyDec_Trunc(PyObject *self, PyObject *dummy UNUSED)
{
	mpd_context_t *ctx;

	CURRENT_CONTEXT_ADDR(ctx);
	return _PyInt_FromDec((PyDecObject *)self, ctx, MPD_ROUND_DOWN);
}

static PyObject *
PyDec_ToIntegralValue(PyObject *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {"rounding", "context", NULL};
	PyDecObject *result;
	PyObject *ctxobj;
	uint32_t status = 0;
	mpd_context_t *ctx;
	mpd_context_t workctx;
	int round = -1;

	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iO", kwlist,
	                                 &round, &ctxobj)) {
		return NULL;
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	workctx = *ctx;
	if (round >= 0) {
		if (!mpd_qsetround(&workctx, round)) {
			return value_error_ptr(invalid_rounding_err);
		}
	}

	if ((result = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qround_to_int(result->dec, DecAddr(self), &workctx, &status);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *) result;
}

static PyObject *
PyDec_ToIntegralExact(PyObject *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {"rounding", "context", NULL};
	PyDecObject *result;
	PyObject *ctxobj;
	uint32_t status = 0;
	mpd_context_t *ctx;
	mpd_context_t workctx;
	int round = -1;

	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iO", kwlist,
	                                 &round, &ctxobj)) {
		return NULL;
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	workctx = *ctx;
	if (round >= 0) {
		if (!mpd_qsetround(&workctx, round)) {
			return value_error_ptr(invalid_rounding_err);
		}
	}

	if ((result = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qround_to_intx(result->dec, DecAddr(self), &workctx, &status);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result);
		return NULL;
	}

	return (PyObject *) result;
}

/* Caller guarantees type */
static PyObject *
PyDec_AsTuple(PyObject *self, PyObject *dummy UNUSED)
{
	PyObject *o_sign = NULL;
	PyObject *o_coeff = NULL;
	PyObject *o_exp = NULL;
	PyObject *o_tuple = NULL;
	PyObject *tmp = NULL;
	mpd_t *selfcpy = NULL;
	char *intstring = NULL;
	Py_ssize_t intlen, i;


	if ((selfcpy = mpd_qncopy(DecAddr(self))) == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		goto error; /* GCOV_UNLIKELY */
	}

	o_sign = Py_BuildValue("i", mpd_sign(DecAddr(self)));
	if (o_sign == NULL) goto error;

	if (mpd_isinfinite(selfcpy)) {
		if ((o_exp = Py_BuildValue("s", "F")) == NULL) {
			goto error; /* GCOV_UNLIKELY */
		}
		if ((o_coeff = PyTuple_New(0)) == NULL) {
			goto error; /* GCOV_UNLIKELY */
		}
	}
	else {
		if (mpd_isnan(selfcpy)) {
			o_exp = Py_BuildValue("s",
			            mpd_isqnan(selfcpy)?"n":"N");
		}
		else {
			o_exp = Py_BuildValue(CONV_mpd_ssize_t,
			            DecAddr(self)->exp);
		}
		if (o_exp == NULL) goto error;

		/* coefficient is defined */
		if (selfcpy->len > 0) {

			/* make an integer */
			selfcpy->exp = 0;
			/* clear NaN and sign */
			mpd_clear_flags(selfcpy);
			intstring = mpd_to_sci(selfcpy, 1);
			if (intstring == NULL) {
				PyErr_NoMemory(); /* GCOV_UNLIKELY */
				goto error; /* GCOV_UNLIKELY */
			}

			intlen = strlen(intstring);
			if ((o_coeff = PyTuple_New(intlen)) == NULL) {
				goto error; /* GCOV_UNLIKELY */
			}

			for (i = 0; i < intlen; i++) {
				tmp = Py_BuildValue("i", intstring[i]-'0');
				if (tmp == NULL) goto error;
				PyTuple_SET_ITEM(o_coeff, i, tmp);
			}
		}
		else {
			if ((o_coeff = PyTuple_New(0)) == NULL) {
				goto error; /* GCOV_UNLIKELY */
			}
		}
	}

	if ((o_tuple = PyTuple_New(3)) == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	PyTuple_SET_ITEM(o_tuple, 0, o_sign);
	PyTuple_SET_ITEM(o_tuple, 1, o_coeff);
	PyTuple_SET_ITEM(o_tuple, 2, o_exp);


out:
	if (selfcpy) mpd_del(selfcpy);
	if (intstring) mpd_free(intstring);
	return o_tuple;

error: /* GCOV_UNLIKELY */
	if (o_sign) { Py_DECREF(o_sign); } /* GCOV_UNLIKELY */
	if (o_coeff) { Py_DECREF(o_coeff); } /* GCOV_UNLIKELY */
	if (o_exp) { Py_DECREF(o_exp); } /* GCOV_UNLIKELY */
	goto out; /* GCOV_UNLIKELY */
}

/* Caller guarantees type. Uses default module context. */
static PyObject *
dec_str(PyDecObject *self)
{
	PyObject *s, *c;
	char *res;

	CURRENT_CONTEXT(c);
	res = mpd_to_sci(self->dec, CtxCaps(c));
	if (res == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	s = PyUnicode_FromString(res);
	mpd_free(res);

	return s;
}

static const char *dtag = "Decimal('";
static const size_t dtaglen = 9; /* without NUL terminator */

/* Caller guarantees type. Uses default module context. */
static PyObject *
dec_repr(PyDecObject *self)
{
	PyObject *s, *c;
	uint8_t err;
	char *cp;
	size_t declen;

	CURRENT_CONTEXT(c);
	cp = mpd_to_sci(self->dec, CtxCaps(c));
	if (cp == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	declen = strlen(cp);

	err = 0;
	cp = mpd_realloc(cp, (mpd_size_t)(declen+dtaglen+3), sizeof *cp, &err);
	if (err) {
		mpd_free(cp); /* GCOV_UNLIKELY */
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	memmove(cp+dtaglen, cp, declen);
	memcpy(cp, dtag, dtaglen);
	cp[declen+dtaglen] = '\'';
	cp[declen+dtaglen+1] = ')';
	cp[declen+dtaglen+2] = '\0';

	s = PyUnicode_FromString(cp);

	mpd_free(cp);
	return s;
}

/* Caller guarantees type. Uses default module context. */
static PyObject *
PyFloat_FromDec(PyDecObject *self)
{
	PyObject *f, *s;

	if ((s = dec_str(self)) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	f = PyFloat_FromString(s);
	Py_DECREF(s);

	return f;
}

static PyObject *
PyDec_Round(PyObject *self, PyObject *args)
{
	PyDecObject *a = (PyDecObject *)self;
	PyDecObject *result;
	PyObject *x = NULL;
	uint32_t status = 0;
	mpd_context_t *ctx;


	CURRENT_CONTEXT_ADDR(ctx);
	if (!PyArg_ParseTuple(args, "|O", &x)) {
		return NULL;
	}

	if (x) {
		mpd_uint_t dq[1] = {1};
		mpd_t q = {MPD_STATIC|MPD_CONST_DATA,0,1,1,1,dq};
		mpd_ssize_t y;

		if (!PyLong_Check(x)) {
			PyErr_SetString(PyExc_TypeError,
			    "optional arg must be an integer.");
			return NULL;
		}

		y = PyLong_AsMpdSsize(x);
		if (PyErr_Occurred()) {
			return NULL;
		}
		if ((result = dec_alloc()) == NULL) {
			return NULL; /* GCOV_UNLIKELY */
		}

		q.exp = (y == MPD_SSIZE_MIN) ? MPD_SSIZE_MAX : -y;
		mpd_qquantize(result->dec, a->dec, &q, ctx, &status);
		if (dec_addstatus(ctx, status)) {
			Py_DECREF(result);
			return NULL;
		}

		return (PyObject *) result;
	}
	else {
		return _PyInt_FromDec(a, ctx, MPD_ROUND_HALF_EVEN);
	}
}

static PyObject *
dec_format(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	PyObject *override = NULL;
	PyObject *dot = NULL;
	PyObject *sep = NULL;
	PyObject *grouping = NULL;
	PyObject *fmtarg, *fmt;
	PyObject *tmp;
	PyObject *ctxobj;
	mpd_spec_t spec;
	wchar_t buf[2];
	char *decstring= NULL;
	uint32_t status = 0;
	size_t n;


	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTuple(args, "O|O", &fmtarg, &override)) {
		return NULL;
	}

	if (PyBytes_Check(fmtarg)) {
		fmt = fmtarg; /* GCOV_NOT_REACHED */
	}
	else if (PyUnicode_Check(fmtarg)) {
		if ((fmt = PyUnicode_AsUTF8String(fmtarg)) == NULL) {
			return NULL; /* GCOV_UNLIKELY */
		}
	}
	else {
		PyErr_SetString(PyExc_TypeError,
		    "format requires bytes or unicode arg.");
		return NULL;
	}

	if (!mpd_parse_fmt_str(&spec, PyBytes_AS_STRING(fmt),
	                       CtxCaps(ctxobj))) {
		PyErr_SetString(PyExc_ValueError,
		    "invalid format string.");
		goto finish;
	}
	if (override) {
		if (!PyDict_Check(override)) {
			PyErr_SetString(PyExc_TypeError,
			    "optional argument must be a dict.");
			goto finish;
		}
		if ((dot = PyDict_GetItemString(override, "decimal_point"))) {
			if ((dot = PyUnicode_AsUTF8String(dot)) == NULL) {
				goto finish; /* GCOV_UNLIKELY */
			}
			spec.dot = PyBytes_AS_STRING(dot);
		}
		if ((sep = PyDict_GetItemString(override, "thousands_sep"))) {
			if ((sep = PyUnicode_AsUTF8String(sep)) == NULL) {
				goto finish; /* GCOV_UNLIKELY */
			}
			spec.sep = PyBytes_AS_STRING(sep);
		}
		if ((grouping = PyDict_GetItemString(override, "grouping"))) {
			if ((grouping = PyUnicode_AsUTF8String(grouping)) == NULL) {
				goto finish; /* GCOV_UNLIKELY */
			}
			spec.grouping = PyBytes_AS_STRING(grouping);
		}
	}
	else {
		n = strlen(spec.dot);
		if (n > 1 || (n == 1 && !isascii((uchar)spec.dot[0]))) {
			n = mbstowcs(buf, spec.dot, 2); /* GCOV_UNLIKELY */
			if (n != 1) { /* GCOV_UNLIKELY */
				PyErr_SetString(PyExc_ValueError, /* GCOV_UNLIKELY */
				    "invalid decimal point or unsupported "
				    "combination of LC_CTYPE and LC_NUMERIC.");
				goto finish; /* GCOV_UNLIKELY */
			}
			if ((tmp = PyUnicode_FromWideChar(buf, n)) == NULL) { /* GCOV_UNLIKELY */
				goto finish; /* GCOV_UNLIKELY */
			}
			if ((dot = PyUnicode_AsUTF8String(tmp)) == NULL) { /* GCOV_UNLIKELY */
				Py_DECREF(tmp); /* GCOV_UNLIKELY */
				goto finish; /* GCOV_UNLIKELY */
			}
			spec.dot = PyBytes_AS_STRING(dot); /* GCOV_UNLIKELY */
			Py_DECREF(tmp); /* GCOV_UNLIKELY */
		}
		n = strlen(spec.sep);
		if (n > 1 || (n == 1 && !isascii((uchar)spec.sep[0]))) {
			n = mbstowcs(buf, spec.sep, 2);
			if (n != 1) {
				PyErr_SetString(PyExc_ValueError, /* GCOV_UNLIKELY */
				    "invalid thousands separator or unsupported "
				    "combination of LC_CTYPE and LC_NUMERIC.");
				goto finish; /* GCOV_UNLIKELY */
			}
			if ((tmp = PyUnicode_FromWideChar(buf, n)) == NULL) {
				goto finish; /* GCOV_UNLIKELY */
			}
			if ((sep = PyUnicode_AsUTF8String(tmp)) == NULL) {
				Py_DECREF(tmp); /* GCOV_UNLIKELY */
				goto finish; /* GCOV_UNLIKELY */
			}
			spec.sep = PyBytes_AS_STRING(sep);
			Py_DECREF(tmp);
		}
	}


	decstring = mpd_qformat_spec(DecAddr(self), &spec, CtxAddr(ctxobj), &status);
	if (decstring == NULL) {
		dec_addstatus(CtxAddr(ctxobj), status);
		goto finish;
	}
	result = PyUnicode_DecodeUTF8(decstring, strlen(decstring), NULL);


finish:
	if (grouping) { Py_DECREF(grouping); }
	if (sep) { Py_DECREF(sep); }
	if (dot) { Py_DECREF(dot); }
	if (decstring) mpd_free(decstring);
	if (fmt != fmtarg) { Py_DECREF(fmt); }
	return result;
}


/********************************************************************/
/*   Macros for converting mpdecimal functions to Decimal methods   */
/********************************************************************/

/* The operand is guaranteed to be a PyDecObject. */
#define _Dec_BoolFunc(MPDFUNC) \
static PyObject *                                                    \
_Dec_##MPDFUNC(PyObject *self)                                       \
{                                                                    \
        PyDecObject *a = (PyDecObject *) self;                       \
        return MPDFUNC(a->dec) ? Dec_INCREF_TRUE : Dec_INCREF_FALSE; \
}

/* The operand is guaranteed to be a PyDecObject. */
#define _Dec_BoolCFunc(MPDFUNC) \
static PyObject *                                                    \
_Dec_CFunc_##MPDFUNC(PyObject *self, PyObject *dummy UNUSED)         \
{                                                                    \
        PyDecObject *a = (PyDecObject *) self;                       \
        return MPDFUNC(a->dec) ? Dec_INCREF_TRUE : Dec_INCREF_FALSE; \
}

/* Operand is a PyDecObject. Uses optional context if supplied.
 * MPDFUNC uses a const context and does not raise. */
#define _DecOpt_BoolFunc(MPDFUNC) \
static PyObject *                                                         \
_DecOpt_##MPDFUNC(PyObject *self, PyObject *args)                         \
{                                                                         \
        PyDecObject *a = (PyDecObject *) self;                            \
        PyObject *ctxobj;                                                 \
        mpd_context_t *ctx;                                               \
                                                                          \
        CURRENT_CONTEXT(ctxobj);                                          \
        if (!PyArg_ParseTuple(args, "|O", &ctxobj)) {                     \
                return NULL;                                              \
        }                                                                 \
                                                                          \
        CONTEXT_CHECK_VA(ctxobj);                                         \
        ctx = CtxAddr(ctxobj);                                            \
                                                                          \
        return MPDFUNC(a->dec, ctx) ? Dec_INCREF_TRUE : Dec_INCREF_FALSE; \
}

/* Operand is a PyDecObject. Uses the default module context.
 * MPDFUNC is a quiet function. */
#define _Dec_UnaryFunc(MPDFUNC) \
static PyObject *                                   \
_Dec_##MPDFUNC(PyObject *self)                      \
{                                                   \
        PyDecObject *a = (PyDecObject *) self;      \
        PyDecObject *result;                        \
        uint32_t status = 0;                        \
        mpd_context_t *ctx;                         \
                                                    \
        CURRENT_CONTEXT_ADDR(ctx);                  \
        if ((result = dec_alloc()) == NULL) {       \
                return NULL;                        \
        }                                           \
                                                    \
        MPDFUNC(result->dec, a->dec, ctx, &status); \
        if (dec_addstatus(ctx, status)) {           \
                Py_DECREF(result);                  \
                return NULL;                        \
        }                                           \
                                                    \
        return (PyObject *) result;                 \
}

/* Operand is a PyDecObject. Uses the default module context.
 * MPDFUNC is a quiet function. */
#define _Dec_UnaryCFunc(MPDFUNC) \
static PyObject *                                            \
_Dec_CFunc_##MPDFUNC(PyObject *self, PyObject *dummy UNUSED) \
{                                                            \
        PyDecObject *a = (PyDecObject *) self;               \
        PyDecObject *result;                                 \
        uint32_t status = 0;                                 \
        mpd_context_t *ctx;                                  \
                                                             \
        CURRENT_CONTEXT_ADDR(ctx);                           \
        if ((result = dec_alloc()) == NULL) {                \
                return NULL;                                 \
        }                                                    \
                                                             \
        MPDFUNC(result->dec, a->dec, ctx, &status);          \
        if (dec_addstatus(ctx, &status)) {                   \
                Py_DECREF(result);                           \
                return NULL;                                 \
        }                                                    \
                                                             \
        return (PyObject *) result;                          \
}

/* Operand is a PyDecObject. Uses optional context if supplied.
 * MPDFUNC is a quiet function. */
#define _DecOpt_UnaryFunc(MPDFUNC) \
static PyObject *                                     \
_DecOpt_##MPDFUNC(PyObject *self, PyObject *args)     \
{                                                     \
        PyDecObject *a = (PyDecObject *) self;        \
        PyDecObject *result;                          \
        PyObject *ctxobj;                             \
        uint32_t status = 0;                          \
        mpd_context_t *ctx;                           \
                                                      \
        CURRENT_CONTEXT(ctxobj);                      \
        if (!PyArg_ParseTuple(args, "|O", &ctxobj)) { \
                return NULL;                          \
        }                                             \
                                                      \
        CONTEXT_CHECK_VA(ctxobj);                     \
        ctx = CtxAddr(ctxobj);                        \
                                                      \
        if ((result = dec_alloc()) == NULL) {         \
                return NULL;                          \
        }                                             \
                                                      \
        MPDFUNC(result->dec, a->dec, ctx, &status);   \
        if (dec_addstatus(ctx, status)) {             \
                Py_DECREF(result);                    \
                return NULL;                          \
        }                                             \
                                                      \
        return (PyObject *) result;                   \
}


/* Operands are Python Objects. Uses default module context.
 * MPDFUNC is a quiet function. */
#define _Dec_BinaryFunc(MPDFUNC)  \
static PyObject *                                           \
_Dec_##MPDFUNC(PyObject *v, PyObject *w)                    \
{                                                           \
        PyDecObject *a, *b;                                 \
        PyDecObject *result;                                \
        uint32_t status = 0;                                \
        mpd_context_t *ctx;                                 \
                                                            \
        CURRENT_CONTEXT_ADDR(ctx);                          \
        CONVERT_BINOP(v, w, &a, &b, ctx);                   \
                                                            \
        if ((result = dec_alloc()) == NULL) {               \
                Py_DECREF(a);                               \
                Py_DECREF(b);                               \
                return NULL;                                \
        }                                                   \
                                                            \
        MPDFUNC(result->dec, a->dec, b->dec, ctx, &status); \
        Py_DECREF(a);                                       \
        Py_DECREF(b);                                       \
        if (dec_addstatus(ctx, status)) {                   \
                Py_DECREF(result);                          \
                return NULL;                                \
        }                                                   \
                                                            \
        return (PyObject *) result;                         \
}

/* Operands are Python Objects. Uses optional context if supplied.
 * MPDFUNC is a quiet function. */
#define _DecOpt_BinaryFunc(MPDFUNC) \
static PyObject *                                           \
_DecOpt_##MPDFUNC(PyObject *v, PyObject *args)              \
{                                                           \
        PyObject *w, *ctxobj;                               \
        PyDecObject *a, *b;                                 \
        PyDecObject *result;                                \
        uint32_t status = 0;                                \
        mpd_context_t *ctx;                                 \
                                                            \
        CURRENT_CONTEXT(ctxobj);                            \
        if (!PyArg_ParseTuple(args, "O|O", &w, &ctxobj)) {  \
                return NULL;                                \
        }                                                   \
                                                            \
        CONTEXT_CHECK_VA(ctxobj);                           \
        ctx = CtxAddr(ctxobj);                              \
                                                            \
        CONVERT_BINOP_SET(v, w, &a, &b, ctx);               \
                                                            \
        if ((result = dec_alloc()) == NULL) {               \
                Py_DECREF(a);                               \
                Py_DECREF(b);                               \
                return NULL;                                \
        }                                                   \
                                                            \
        MPDFUNC(result->dec, a->dec, b->dec, ctx, &status); \
        Py_DECREF(a);                                       \
        Py_DECREF(b);                                       \
        if (dec_addstatus(ctx, status)) {                   \
                Py_DECREF(result);                          \
                return NULL;                                \
        }                                                   \
                                                            \
        return (PyObject *) result;                         \
}

/* Operands are Python Objects. Actual MPDFUNC does NOT take a context.
 * Uses optional context for conversion only. */
#define _DecOpt_BinaryFunc_NoCtx(MPDFUNC) \
static PyObject *                                          \
_DecOpt_##MPDFUNC(PyObject *v, PyObject *args)             \
{                                                          \
        PyObject *w, *ctxobj;                              \
        PyDecObject *a, *b;                                \
        PyDecObject *result;                               \
        mpd_context_t *ctx;                                \
                                                           \
        CURRENT_CONTEXT(ctxobj);                           \
        if (!PyArg_ParseTuple(args, "O|O", &w, &ctxobj)) { \
                return NULL;                               \
        }                                                  \
                                                           \
        CONTEXT_CHECK_VA(ctxobj);                          \
        ctx = CtxAddr(ctxobj);                             \
                                                           \
        CONVERT_BINOP_SET(v, w, &a, &b, ctx);              \
                                                           \
        if ((result = dec_alloc()) == NULL) {              \
                Py_DECREF(a);                              \
                Py_DECREF(b);                              \
                return NULL;                               \
        }                                                  \
                                                           \
        MPDFUNC(result->dec, a->dec, b->dec);              \
        Py_DECREF(a);                                      \
        Py_DECREF(b);                                      \
                                                           \
        return (PyObject *) result;                        \
}


/* Operands are Python Objects. Uses optional context if supplied.
 * MPDFUNC is a quiet function. */
#define _DecOpt_TernaryFunc(MPDFUNC) \
static PyObject *                                                   \
_DecOpt_##MPDFUNC(PyObject *v, PyObject *args)                      \
{                                                                   \
        PyObject *w, *x, *ctxobj;                                   \
        PyDecObject *a, *b, *c;                                     \
        PyDecObject *result;                                        \
        uint32_t status = 0;                                        \
        mpd_context_t *ctx;                                         \
                                                                    \
        CURRENT_CONTEXT(ctxobj);                                    \
        if (!PyArg_ParseTuple(args, "OO|O", &w, &x, &ctxobj)) {     \
                return NULL;                                        \
        }                                                           \
                                                                    \
        CONTEXT_CHECK_VA(ctxobj);                                   \
        ctx = CtxAddr(ctxobj);                                      \
                                                                    \
        CONVERT_TERNOP_SET(v, w, x, &a, &b, &c, ctx);               \
                                                                    \
        if ((result = dec_alloc()) == NULL) {                       \
                Py_DECREF(a);                                       \
                Py_DECREF(b);                                       \
                Py_DECREF(c);                                       \
                return NULL;                                        \
        }                                                           \
                                                                    \
        MPDFUNC(result->dec, a->dec, b->dec, c->dec, ctx, &status); \
        Py_DECREF(a);                                               \
        Py_DECREF(b);                                               \
        Py_DECREF(c);                                               \
        if (dec_addstatus(ctx, status)) {                           \
                Py_DECREF(result);                                  \
                return NULL;                                        \
        }                                                           \
                                                                    \
        return (PyObject *) result;                                 \
}


/**********************************************/
/*              Number methods                */
/**********************************************/

_Dec_UnaryFunc(mpd_qminus)
_Dec_UnaryFunc(mpd_qplus)
_Dec_UnaryFunc(mpd_qabs)

static PyObject *
_Dec_mpd_adjexp(PyObject *self, PyObject *dummy UNUSED)
{
	mpd_ssize_t retval;

	if (mpd_isspecial(DecAddr(self))) {
		retval = 0;
	}
	else {
		retval = mpd_adjexp(DecAddr(self));
	}

	return _PyLong_FromMpdSsize(retval);
}

_Dec_BinaryFunc(mpd_qadd)
_Dec_BinaryFunc(mpd_qsub)
_Dec_BinaryFunc(mpd_qmul)
_Dec_BinaryFunc(mpd_qdiv)
_Dec_BinaryFunc(mpd_qrem)
_Dec_BinaryFunc(mpd_qdivint)

static int
_Dec_nonzero(PyDecObject *v)
{
	return !mpd_iszero(v->dec);
}

static PyObject *
_Dec_mpd_qdivmod(PyObject *v, PyObject *w)
{
	PyDecObject *a, *b;
	PyDecObject *q, *r;
	uint32_t status = 0;
	mpd_context_t *ctx;

	CURRENT_CONTEXT_ADDR(ctx);
	CONVERT_BINOP(v, w, &a, &b, ctx);

	if ((q = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((r = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		Py_DECREF(q); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qdivmod(q->dec, r->dec, a->dec, b->dec, ctx, &status);
	Py_DECREF(a);
	Py_DECREF(b);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(r);
		Py_DECREF(q);
		return NULL;
	}

	return Py_BuildValue("(NN)", q, r);
}

static PyObject *
_Dec_mpd_qpow(PyObject *base, PyObject *exp, PyObject *mod)
{
	PyDecObject *a, *b, *c = NULL;
	PyDecObject *result;
	uint32_t status = 0;
	mpd_context_t *ctx;

	CURRENT_CONTEXT_ADDR(ctx);
	CONVERT_BINOP(base, exp, &a, &b, ctx);

	if (mod != Py_None) {
		if (!convert_op(mod, &c, ctx)) {
			Py_DECREF(a);
			Py_DECREF(b);
			return (PyObject *) c;
		}
	}

	if ((result = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	if (c == NULL) {
		mpd_qpow(result->dec, a->dec, b->dec, ctx, &status);
	}
	else {
		mpd_qpowmod(result->dec, a->dec, b->dec, c->dec, ctx, &status);
		Py_DECREF(c);
	}
	Py_DECREF(a);
	Py_DECREF(b);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result);
		return NULL;
	}

	return (PyObject *) result;
}


/******************************************************************************/
/*                             Decimal Methods                                */
/******************************************************************************/


/* Arithmetic operations */
_DecOpt_UnaryFunc(mpd_qabs)
_DecOpt_UnaryFunc(mpd_qexp)
_DecOpt_UnaryFunc(mpd_qinvroot)
_DecOpt_UnaryFunc(mpd_qln)
_DecOpt_UnaryFunc(mpd_qlog10)
_DecOpt_UnaryFunc(mpd_qminus)
_DecOpt_UnaryFunc(mpd_qnext_minus)
_DecOpt_UnaryFunc(mpd_qnext_plus)
_DecOpt_UnaryFunc(mpd_qplus)
_DecOpt_UnaryFunc(mpd_qreduce)
_DecOpt_UnaryFunc(mpd_qsqrt)

_DecOpt_BinaryFunc(mpd_qadd)
_DecOpt_BinaryFunc(mpd_qcompare)
_DecOpt_BinaryFunc(mpd_qcompare_signal)
_DecOpt_BinaryFunc(mpd_qdiv)
_DecOpt_BinaryFunc(mpd_qdivint)
_DecOpt_BinaryFunc(mpd_qmax)
_DecOpt_BinaryFunc(mpd_qmax_mag)
_DecOpt_BinaryFunc(mpd_qmin)
_DecOpt_BinaryFunc(mpd_qmin_mag)
_DecOpt_BinaryFunc(mpd_qmul)
_DecOpt_BinaryFunc(mpd_qnext_toward)
_DecOpt_BinaryFunc(mpd_qpow)
_DecOpt_BinaryFunc(mpd_qrem)
_DecOpt_BinaryFunc(mpd_qrem_near)
_DecOpt_BinaryFunc(mpd_qsub)

_DecOpt_TernaryFunc(mpd_qfma)
_DecOpt_TernaryFunc(mpd_qpowmod)

/* Miscellaneous */
_Dec_BoolCFunc(mpd_iscanonical)
_Dec_BoolCFunc(mpd_isfinite)
_Dec_BoolCFunc(mpd_isinfinite)
_Dec_BoolCFunc(mpd_isinteger)
_Dec_BoolCFunc(mpd_isnan)
_Dec_BoolCFunc(mpd_isqnan)
_Dec_BoolCFunc(mpd_issnan)
_Dec_BoolCFunc(mpd_issigned)
_Dec_BoolCFunc(mpd_isspecial)
_Dec_BoolCFunc(mpd_iszero)

_DecOpt_BoolFunc(mpd_isnormal)
_DecOpt_BoolFunc(mpd_issubnormal)

static PyObject *
_Dec_mpd_qcopy_abs(PyObject *self, PyObject *dummy UNUSED)
{
	PyDecObject *a = (PyDecObject *) self;
	PyDecObject *result;
	uint32_t status = 0;
	mpd_context_t *ctx;

	CURRENT_CONTEXT_ADDR(ctx);
	if ((result = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qcopy_abs(result->dec, a->dec, &status);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *) result;
}

static PyObject *
_Dec_mpd_qcopy_negate(PyObject *self, PyObject *dummy UNUSED)
{
	PyDecObject *a = (PyDecObject *) self;
	PyDecObject *result;
	uint32_t status = 0;
	mpd_context_t *ctx;

	CURRENT_CONTEXT_ADDR(ctx);
	if ((result = dec_alloc()) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qcopy_negate(result->dec, a->dec, &status);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *) result;
}

_DecOpt_UnaryFunc(mpd_qinvert)
_DecOpt_UnaryFunc(mpd_qlogb)

_DecOpt_BinaryFunc_NoCtx(mpd_compare_total)
_DecOpt_BinaryFunc_NoCtx(mpd_compare_total_mag)

static PyObject *
_Dec_mpd_qcopy_sign(PyObject *v, PyObject *w)
{
	PyDecObject *a, *b;
	PyDecObject *result;
	uint32_t status = 0;
	mpd_context_t *ctx;

	CURRENT_CONTEXT_ADDR(ctx);
	CONVERT_BINOP_SET(v, w, &a, &b, ctx);

	if ((result = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qcopy_sign(result->dec, a->dec, b->dec, &status);
	Py_DECREF(a);
	Py_DECREF(b);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *) result;
}

_DecOpt_BinaryFunc(mpd_qand)
_DecOpt_BinaryFunc(mpd_qor)
_DecOpt_BinaryFunc(mpd_qxor)

_DecOpt_BinaryFunc(mpd_qrotate)
_DecOpt_BinaryFunc(mpd_qscaleb)
_DecOpt_BinaryFunc(mpd_qshift)

static PyObject *
_DecOpt_mpd_class(PyObject *self, PyObject *args)
{
	PyDecObject *a = (PyDecObject *) self;
	PyObject *ctxobj;
	mpd_context_t *ctx;
	const char *cp;

	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTuple(args, "|O", &ctxobj)) {
		return NULL;
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	cp = mpd_class(a->dec, ctx);
	return Py_BuildValue("s", cp);
}

static PyObject *
_Dec_canonical(PyObject *self, PyObject *dummy UNUSED)
{
	Py_INCREF(self);
	return self;
}

static PyObject *
dec_copy(PyObject *self, PyObject *dummy UNUSED)
{
	Py_INCREF(self);
	return self;
}

static PyObject *
_DecOpt_mpd_qdivmod(PyObject *v, PyObject *args)
{
	PyObject *w, *ctxobj;
	PyDecObject *a, *b;
	PyDecObject *q, *r;
	uint32_t status = 0;
	mpd_context_t *ctx;

	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTuple(args, "O|O", &w, &ctxobj)) {
		return NULL;
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	CONVERT_BINOP_SET(v, w, &a, &b, ctx);

	if ((q = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((r = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		Py_DECREF(q); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qdivmod(q->dec, r->dec, a->dec, b->dec, ctx, &status);
	Py_DECREF(a);
	Py_DECREF(b);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(r);
		Py_DECREF(q);
		return NULL;
	}

	return Py_BuildValue("(NN)", q, r);
}

static PyObject *
_Dec_mpd_qquantize(PyObject *v, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {"exp", "rounding", "context", NULL};
	PyObject *w, *ctxobj;
	PyDecObject *a, *b;
	PyDecObject *result;
	uint32_t status = 0;
	mpd_context_t *ctx;
	mpd_context_t workctx;
	int round = -1;

	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|iO", kwlist,
	                                 &w, &round, &ctxobj)) {
		return NULL;
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	workctx = *ctx;
	if (round >= 0) {
		if (!mpd_qsetround(&workctx, round)) {
			return value_error_ptr(invalid_rounding_err);
		}
	}

	CONVERT_BINOP_SET(v, w, &a, &b, ctx);

	if ((result = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qquantize(result->dec, a->dec, b->dec, &workctx, &status);
	Py_DECREF(a);
	Py_DECREF(b);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result);
		return NULL;
	}

	return (PyObject *) result;
}

static PyObject *
_Dec_mpd_radix(PyObject *self UNUSED, PyObject *dummy UNUSED)
{
	return Py_BuildValue("i", 10);
}

static PyObject *
_Dec_mpd_same_quantum(PyObject *v, PyObject *args)
{
	PyObject *w, *ctxobj, *result;
	PyDecObject *a, *b;
	mpd_context_t *ctx;

	CURRENT_CONTEXT(ctxobj);
	if (!PyArg_ParseTuple(args, "O|O", &w, &ctxobj)) {
		return NULL;
	}

	CONTEXT_CHECK_VA(ctxobj);
	ctx = CtxAddr(ctxobj);

	CONVERT_BINOP_SET(v, w, &a, &b, ctx);

	result = mpd_same_quantum(a->dec, b->dec) ?
	             Dec_INCREF_TRUE : Dec_INCREF_FALSE;
	Py_DECREF(a);
	Py_DECREF(b);

	return result;
}

static PyObject *
_Dec_mpd_sign(PyObject *self, PyObject *dummy UNUSED)
{
	PyDecObject *a = (PyDecObject *) self;

	return Py_BuildValue("i", mpd_arith_sign(a->dec));
}

static PyObject *
_Dec_mpd_to_sci(PyObject *self, PyObject *args)
{
	PyDecObject *a = (PyDecObject *)self;
	PyObject *result, *c;
	char *s;

	CURRENT_CONTEXT(c);
	if (!PyArg_ParseTuple(args, "|O", &c)) {
		return NULL;
	}
	CONTEXT_CHECK_VA(c);

	s = mpd_to_sci(a->dec, CtxCaps(c));
	if (s == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	result = PyUnicode_FromString(s);
	mpd_free(s);

	return result;
}

static PyObject *
_Dec_mpd_to_eng(PyObject *self, PyObject *args)
{
	PyDecObject *a = (PyDecObject *)self;
	PyObject *result, *c;
	char *s;

	CURRENT_CONTEXT(c);
	if (!PyArg_ParseTuple(args, "|O", &c)) {
		return NULL;
	}
	CONTEXT_CHECK_VA(c);

	s = mpd_to_eng(a->dec, CtxCaps(c));
	if (s == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	result = PyUnicode_FromString(s);
	mpd_free(s);

	return result;
}

static PyObject *
dec_richcompare(PyObject *v, PyObject *w, int op)
{
	PyDecObject *a = (PyDecObject *)v;
	PyDecObject *b;
	uint32_t status = 0;
	mpd_context_t *ctx;
	int a_issnan, b_issnan;
	int r;

	CURRENT_CONTEXT_ADDR(ctx);
	CONVERT_BINOP_CMP(v, w, &a, &b, op, ctx);

	a_issnan = mpd_issnan(a->dec);
	b_issnan = mpd_issnan(b->dec);

	r = mpd_qcmp(a->dec, b->dec, &status);
	Py_DECREF(a);
	Py_DECREF(b);
	if (r == INT_MAX) {
		/* sNaNs or op={le,ge,lt,gt} always signal. */
		if (a_issnan || b_issnan || (op != Py_EQ && op != Py_NE)) {
			if (dec_addstatus(ctx, status)) {
				return NULL;
			}
		}
		/* qNaN comparison with op={eq,ne} or comparison
		 * with InvalidOperation disabled. */
		return (op == Py_NE) ? Dec_INCREF_TRUE : Dec_INCREF_FALSE;
	}

	switch (op) {
	case Py_EQ:
		r = (r == 0);
		break;
	case Py_NE:
		r = (r != 0);
		break;
	case Py_LE:
		r = (r <= 0);
		break;
	case Py_GE:
		r = (r >= 0);
		break;
	case Py_LT:
		r = (r == -1);
		break;
	case Py_GT:
		r = (r == 1);
		break;
	}

	return PyBool_FromLong(r);
}

#if PY_VERSION_HEX < 0x03020000
/* Always uses the module context */
static long
dec_hash(PyObject *v)
{
#if defined(CONFIG_64)
	mpd_uint_t data_two64m1[2] = {8446744073709551615ULL, 1ULL};
	mpd_t two64m1 = {MPD_POS|MPD_STATIC|MPD_CONST_DATA, 0, 20, 2, 2,
	                 data_two64m1};
#elif defined(CONFIG_32)
	mpd_uint_t data_two64m1[3] = {709551615UL, 446744073UL, 18UL};
	mpd_t two64m1 = {MPD_POS|MPD_STATIC|MPD_CONST_DATA, 0, 20, 3, 2,
	                 data_two64m1};
#else
	#error "CONFIG_64 or CONFIG_32 must be defined."
#endif
	mpd_uint_t data_ten[1] = {10};
	mpd_t ten = {MPD_POS|MPD_STATIC|MPD_CONST_DATA, 0, 2, 1, 1, data_ten};

	PyDecObject *a;
	PyObject *obj = NULL;
	mpd_t *tmp = NULL;
	char *cp = NULL;
	uint32_t status = 0;
	mpd_context_t maxcontext;
	long result;

	if ((a = dec_alloc()) == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return -1; /* GCOV_UNLIKELY */
	}
	if (!mpd_qcopy(a->dec, DecAddr(v), &status)) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		result = -1; /* GCOV_UNLIKELY */
		goto finish; /* GCOV_UNLIKELY */
	}

	if (mpd_isspecial(a->dec)) {
		if (mpd_isnan(a->dec)) {
			PyErr_SetString(PyExc_TypeError,
			    "cannot hash a NaN value");
			result = -1;
		}
		else {
			if ((obj = dec_str(a)) == NULL) {
				result = -1; /* GCOV_UNLIKELY */
				goto finish; /* GCOV_UNLIKELY */
			}
			result = PyObject_Hash(obj);
		}
	}
	else if (mpd_iszero(a->dec)) {
		result = 0;
	}
	else if (mpd_isinteger(a->dec)) {
		PyObject *ctxobj = current_context();
		mpd_context_t *ctx;
		if (ctxobj == NULL) {
			result = -1; /* GCOV_UNLIKELY */
			goto finish; /* GCOV_UNLIKELY */
		}
		ctx = CtxAddr(ctxobj);
		mpd_maxcontext(&maxcontext);

		if ((tmp = mpd_qnew()) == NULL) {
			PyErr_NoMemory(); /* GCOV_UNLIKELY */
			result = -1; /* GCOV_UNLIKELY */
			goto finish; /* GCOV_UNLIKELY */
		}

		/* clobbering a function scope object */
		mpd_qround_to_int(a->dec, a->dec, ctx, &status);
		mpd_qset_ssize(tmp, a->dec->exp, &maxcontext, &status);
		mpd_qpowmod(tmp, &ten, tmp, &two64m1, &maxcontext, &status);
		a->dec->exp = 0;
		mpd_qmul(a->dec, a->dec, tmp, &maxcontext, &status);

		if (status&MPD_Errors) {
			if (dec_addstatus(ctx, status)) { /* GCOV_UNLIKELY */
				result = -1; /* GCOV_UNLIKELY */
				goto finish; /* GCOV_UNLIKELY */
			}
		}

		if ((obj = PyLong_FromDecCtx(a, &maxcontext)) == NULL) {
			result = -1; /* GCOV_UNLIKELY */
			goto finish; /* GCOV_UNLIKELY */
		}
		result = PyObject_Hash(obj);
	}
	else {
		mpd_ssize_t tz;
		mpd_ssize_t exp;
		int sign;

		mpd_maxcontext(&maxcontext);
		tz = mpd_trail_zeros(a->dec);
		exp = a->dec->exp + a->dec->digits;
		sign = mpd_sign(a->dec);

		mpd_qshiftr_inplace(a->dec, tz);
		a->dec->exp = 0;
		mpd_set_flags(a->dec, MPD_POS);

		cp = mpd_to_sci(a->dec, 1);
		obj = Py_BuildValue("(i"CONV_mpd_ssize_t"s)", sign, exp, cp);
		if (obj == NULL) {
			result = -1; /* GCOV_UNLIKELY */
			goto finish; /* GCOV_UNLIKELY */
		}
		result =  PyObject_Hash(obj);
	}


finish:
	Py_DECREF(a);
	if (obj) {Py_DECREF(obj);}
	if (tmp) mpd_del(tmp);
	if (cp) mpd_free(cp);
	return result;
}
#else
/* Always uses the module context */
static Py_hash_t
dec_hash(PyObject *v)
{
#if defined(CONFIG_64) && _PyHASH_BITS == 61
	/* 2**61 - 1 */
	mpd_uint_t p_data[1] = {2305843009213693951ULL};
	mpd_t p = {MPD_POS|MPD_STATIC|MPD_CONST_DATA, 0, 19, 1, 1, p_data};
	/* Inverse of 10 modulo p */
	mpd_uint_t inv10_p_data[2] = {2075258708292324556ULL};
	mpd_t inv10_p = {MPD_POS|MPD_STATIC|MPD_CONST_DATA,
	                 0, 19, 1, 1, inv10_p_data};
#elif defined(CONFIG_32) && _PyHASH_BITS == 31
	/* 2**31 - 1 */
	mpd_uint_t p_data[2] = {147483647UL, 2};
	mpd_t p = {MPD_POS|MPD_STATIC|MPD_CONST_DATA, 0, 10, 2, 2, p_data};
	/* Inverse of 10 modulo p */
	mpd_uint_t inv10_p_data[2] = {503238553UL, 1};
	mpd_t inv10_p = {MPD_POS|MPD_STATIC|MPD_CONST_DATA,
	                 0, 10, 2, 2, inv10_p_data};
#else
	#error "No valid combination of CONFIG_64, CONFIG_32 and _PyHASH_BITS."
#endif
	const Py_hash_t py_hash_inf = 314159;
	const Py_hash_t py_hash_nan = 0;
	mpd_uint_t ten_data[1] = {10};
	mpd_t ten = {MPD_POS|MPD_STATIC|MPD_CONST_DATA,
	             0, 2, 1, 1, ten_data};
	Py_hash_t result;
	mpd_t *exp_hash = NULL;
	mpd_t *tmp = NULL;
	mpd_ssize_t exp;
	uint32_t status = 0;
	mpd_context_t *ctx, maxcontext;
	PyObject *ctxobj;


	ctxobj = current_context();
	if (ctxobj == NULL) {
		return -1; /* GCOV_UNLIKELY */
	}
	ctx = CtxAddr(ctxobj);

	if (mpd_isspecial(DecAddr(v))) {
		if (mpd_issnan(DecAddr(v))) {
			PyErr_SetString(PyExc_TypeError,
			    "Cannot hash a signaling NaN value.");
			return -1;
		}
		else if (mpd_isnan(DecAddr(v))) {
			return py_hash_nan;
		}
		else {
			return py_hash_inf * mpd_arith_sign(DecAddr(v));
		}
	}

	mpd_maxcontext(&maxcontext);
	if ((exp_hash = mpd_qnew()) == NULL) {
		goto malloc_error; /* GCOV_UNLIKELY */
	}
	if ((tmp = mpd_qnew()) == NULL) {
		goto malloc_error; /* GCOV_UNLIKELY */
	}

	/*
	 * exp(v): exponent of v
	 * int(v): coefficient of v
	 */
	exp = DecAddr(v)->exp;
	if (exp >= 0) {
		/* 10**exp(v) % p */
		mpd_qsset_ssize(tmp, exp, &maxcontext, &status);
		mpd_qpowmod(exp_hash, &ten, tmp, &p, &maxcontext, &status);
	}
	else {
		/* inv10_p**(-exp(v)) % p */
		mpd_qsset_ssize(tmp, -exp, &maxcontext, &status);
		mpd_qpowmod(exp_hash, &inv10_p, tmp, &p, &maxcontext, &status);
	}

	/* hash = (int(v) * exp_hash) % p */
	if (!mpd_qcopy(tmp, DecAddr(v), &status)) {
		goto malloc_error; /* GCOV_UNLIKELY */
	}
	tmp->exp = 0;
	mpd_set_positive(tmp);
	mpd_qmul(tmp, tmp, exp_hash, &maxcontext, &status);
	mpd_qrem(tmp, tmp, &p, &maxcontext, &status);

	result = mpd_qget_ssize(tmp, &status);
	result = mpd_ispositive(DecAddr(v)) ? result : -result;
	result = (result == -1) ? -2 : result;

	if (status != 0) {
		status |= MPD_Invalid_operation; /* GCOV_UNLIKELY */
		if (dec_addstatus(ctx, status)) { /* GCOV_UNLIKELY */
			result = -1; /* GCOV_UNLIKELY */
			goto finish; /* GCOV_UNLIKELY */
		}
	}


finish:
	if (exp_hash) mpd_del(exp_hash);
	if (tmp) mpd_del(tmp);
	return result;

malloc_error: /* GCOV_UNLIKELY */
	PyErr_NoMemory(); /* GCOV_UNLIKELY */
	result = -1; /* GCOV_UNLIKELY */
	goto finish; /* GCOV_UNLIKELY */
}
#endif

static PyObject *
dec_reduce(PyObject *self, PyObject *dummy UNUSED)
{
	PyObject *newob, *mpd_str;

	if ((mpd_str = dec_str((PyDecObject *)self)) == NULL) {
		return NULL; /* GCOV_UNLIKELY */
	}

	newob = Py_BuildValue("O(N)", Py_TYPE(self), mpd_str);

	return newob;
}

static PyNumberMethods dec_number_methods =
{
	(binaryfunc) _Dec_mpd_qadd,
	(binaryfunc) _Dec_mpd_qsub,
	(binaryfunc) _Dec_mpd_qmul,
	(binaryfunc) _Dec_mpd_qrem,
	(binaryfunc) _Dec_mpd_qdivmod,
	(ternaryfunc) _Dec_mpd_qpow,
	(unaryfunc) _Dec_mpd_qminus,
	(unaryfunc) _Dec_mpd_qplus,
	(unaryfunc) _Dec_mpd_qabs,
	(inquiry) _Dec_nonzero,
	(unaryfunc) 0,   /* no bit-complement */
	(binaryfunc) 0,  /* no shiftl */
	(binaryfunc) 0,  /* no shiftr */
	(binaryfunc) 0,  /* no bit-and */
	(binaryfunc) 0,  /* no bit-xor */
	(binaryfunc) 0,  /* no bit-ior */
	(unaryfunc) PyLong_FromDec,
	0,               /* nb_reserved */
	(unaryfunc) PyFloat_FromDec,
	0,               /* binaryfunc nb_inplace_add; */
	0,               /* binaryfunc nb_inplace_subtract; */
	0,               /* binaryfunc nb_inplace_multiply; */
	0,               /* binaryfunc nb_inplace_remainder; */
	0,               /* ternaryfunc nb_inplace_power; */
	0,               /* binaryfunc nb_inplace_lshift; */
	0,               /* binaryfunc nb_inplace_rshift; */
	0,               /* binaryfunc nb_inplace_and; */
	0,               /* binaryfunc nb_inplace_xor; */
	0,               /* binaryfunc nb_inplace_or; */
	(binaryfunc) _Dec_mpd_qdivint,  /* binaryfunc nb_floor_divide; */
	(binaryfunc) _Dec_mpd_qdiv,     /* binaryfunc nb_true_divide; */
	0,               /* binaryfunc nb_inplace_floor_divide; */
	0,               /* binaryfunc nb_inplace_true_divide; */
};

static PyMethodDef dec_methods [] =
{
  /* Unary arithmetic functions */
  { "abs", _DecOpt_mpd_qabs, METH_VARARGS, doc_abs },
  { "exp", _DecOpt_mpd_qexp, METH_VARARGS, doc_exp },
  { "invroot", _DecOpt_mpd_qinvroot, METH_VARARGS, doc_invroot },
  { "ln", _DecOpt_mpd_qln, METH_VARARGS, doc_ln },
  { "log10", _DecOpt_mpd_qlog10, METH_VARARGS, doc_log10 },
  { "minus", _DecOpt_mpd_qminus, METH_VARARGS, doc_minus },
  { "next_minus", _DecOpt_mpd_qnext_minus, METH_VARARGS, doc_next_minus },
  { "next_plus", _DecOpt_mpd_qnext_plus, METH_VARARGS, doc_next_plus },
  { "normalize", _DecOpt_mpd_qreduce, METH_VARARGS, doc_normalize }, /* alias for reduce */
  { "plus", _DecOpt_mpd_qplus, METH_VARARGS, doc_plus },
  { "reduce", _DecOpt_mpd_qreduce, METH_VARARGS, doc_reduce },
  { "to_integral", (PyCFunction)PyDec_ToIntegralValue, METH_VARARGS|METH_KEYWORDS, doc_to_integral },
  { "to_integral_exact", (PyCFunction)PyDec_ToIntegralExact, METH_VARARGS|METH_KEYWORDS, doc_to_integral_exact },
  { "to_integral_value", (PyCFunction)PyDec_ToIntegralValue, METH_VARARGS|METH_KEYWORDS, doc_to_integral_value },
  { "sqrt", _DecOpt_mpd_qsqrt, METH_VARARGS, doc_sqrt },

  /* Binary arithmetic functions */
  { "add", _DecOpt_mpd_qadd, METH_VARARGS, doc_add },
  { "compare", _DecOpt_mpd_qcompare, METH_VARARGS, doc_compare },
  { "compare_signal", _DecOpt_mpd_qcompare_signal, METH_VARARGS, doc_compare_signal },
  { "div", _DecOpt_mpd_qdiv, METH_VARARGS, doc_div }, /* alias for divide */
  { "divide", _DecOpt_mpd_qdiv, METH_VARARGS, doc_divide },
  { "divide_int", _DecOpt_mpd_qdivint, METH_VARARGS, doc_divide_int },
  { "divint", _DecOpt_mpd_qdivint, METH_VARARGS, doc_divint }, /* alias for divide_int */
  { "divmod", _DecOpt_mpd_qdivmod, METH_VARARGS, doc_divmod },
  { "max", _DecOpt_mpd_qmax, METH_VARARGS, doc_max },
  { "max_mag", _DecOpt_mpd_qmax_mag, METH_VARARGS, doc_max_mag },
  { "min", _DecOpt_mpd_qmin, METH_VARARGS, doc_min },
  { "min_mag", _DecOpt_mpd_qmin_mag, METH_VARARGS, doc_min_mag },
  { "mul", _DecOpt_mpd_qmul, METH_VARARGS, doc_mul }, /* alias for multiply */
  { "multiply", _DecOpt_mpd_qmul, METH_VARARGS, doc_multiply },
  { "next_toward", _DecOpt_mpd_qnext_toward, METH_VARARGS, doc_next_toward },
  { "pow", _DecOpt_mpd_qpow, METH_VARARGS, doc_pow }, /* alias for power */
  { "power", _DecOpt_mpd_qpow, METH_VARARGS, doc_power },
  { "quantize", (PyCFunction)_Dec_mpd_qquantize, METH_VARARGS|METH_KEYWORDS, doc_quantize },
  { "rem", _DecOpt_mpd_qrem, METH_VARARGS, doc_rem }, /* alias for remainder */
  { "remainder", _DecOpt_mpd_qrem, METH_VARARGS, doc_remainder },
  { "remainder_near", _DecOpt_mpd_qrem_near, METH_VARARGS, doc_remainder_near },
  { "sub", _DecOpt_mpd_qsub, METH_VARARGS, doc_sub }, /* alias for subtract */
  { "subtract", _DecOpt_mpd_qsub, METH_VARARGS, doc_subtract },

  /* Ternary arithmetic functions */
  { "fma", _DecOpt_mpd_qfma, METH_VARARGS, doc_fma },
  { "powmod", _DecOpt_mpd_qpowmod, METH_VARARGS, doc_powmod },

  /* Boolean functions, no context arg */
  { "is_canonical", _Dec_CFunc_mpd_iscanonical, METH_NOARGS, doc_is_canonical },
  { "is_finite", _Dec_CFunc_mpd_isfinite, METH_NOARGS, doc_is_finite },
  { "is_infinite", _Dec_CFunc_mpd_isinfinite, METH_NOARGS, doc_is_infinite },
  { "is_integer", _Dec_CFunc_mpd_isinteger, METH_NOARGS, doc_is_integer },
  { "is_nan", _Dec_CFunc_mpd_isnan, METH_NOARGS, doc_is_nan },
  { "is_qnan", _Dec_CFunc_mpd_isqnan, METH_NOARGS, doc_is_qnan },
  { "is_snan", _Dec_CFunc_mpd_issnan, METH_NOARGS, doc_is_snan },
  { "is_signed", _Dec_CFunc_mpd_issigned, METH_NOARGS, doc_is_signed },
  { "is_special", _Dec_CFunc_mpd_isspecial, METH_NOARGS, doc_is_special },
  { "is_zero", _Dec_CFunc_mpd_iszero, METH_NOARGS, doc_is_zero },

  /* Boolean functions, optional context arg */
  { "is_normal", _DecOpt_mpd_isnormal, METH_VARARGS, doc_is_normal },
  { "is_subnormal", _DecOpt_mpd_issubnormal, METH_VARARGS, doc_is_subnormal },

  /* Unary functions, no context arg */
  { "adjusted", _Dec_mpd_adjexp, METH_NOARGS, doc_adjusted },
  { "canonical", _Dec_canonical, METH_NOARGS, doc_canonical },
  { "copy_abs", _Dec_mpd_qcopy_abs, METH_NOARGS, doc_copy_abs },
  { "copy_negate", _Dec_mpd_qcopy_negate, METH_NOARGS, doc_copy_negate },
  { "radix", _Dec_mpd_radix, METH_NOARGS, doc_radix },
  { "sign", _Dec_mpd_sign, METH_NOARGS, doc_sign },

  /* Unary functions, optional context arg */
  { "apply", PyDec_Apply, METH_VARARGS, doc_apply },
  { "logb", _DecOpt_mpd_qlogb, METH_VARARGS, doc_logb },
  { "logical_invert", _DecOpt_mpd_qinvert, METH_VARARGS, doc_logical_invert },
  { "number_class", _DecOpt_mpd_class, METH_VARARGS, doc_number_class },
  { "to_sci", _Dec_mpd_to_sci, METH_VARARGS, doc_to_sci }, /* alias for to_sci_string */
  { "to_sci_string", _Dec_mpd_to_sci, METH_VARARGS, doc_to_sci_string },
  { "to_eng", _Dec_mpd_to_eng, METH_VARARGS, doc_to_eng }, /* alias for to_eng_string */
  { "to_eng_string", _Dec_mpd_to_eng, METH_VARARGS, doc_to_eng_string },

  /* Binary functions, optional context arg */
  { "compare_total", _DecOpt_mpd_compare_total, METH_VARARGS, doc_compare_total },
  { "compare_total_mag", _DecOpt_mpd_compare_total_mag, METH_VARARGS, doc_compare_total_mag },
  { "copy_sign", _Dec_mpd_qcopy_sign, METH_O, doc_copy_sign },
  { "logical_and", _DecOpt_mpd_qand, METH_VARARGS, doc_logical_and },
  { "logical_or", _DecOpt_mpd_qor, METH_VARARGS, doc_logical_or },
  { "logical_xor", _DecOpt_mpd_qxor, METH_VARARGS, doc_logical_xor },
  { "rotate", _DecOpt_mpd_qrotate, METH_VARARGS, doc_rotate },
  { "same_quantum", _Dec_mpd_same_quantum, METH_VARARGS, doc_same_quantum },
  { "scaleb", _DecOpt_mpd_qscaleb, METH_VARARGS, doc_scaleb },
  { "shift", _DecOpt_mpd_qshift, METH_VARARGS, doc_shift },

  /* Miscellaneous */
  { "from_float", _PyDec_FromFloat_Max, METH_O|METH_CLASS, doc_from_float },
  { "as_tuple", PyDec_AsTuple, METH_NOARGS, doc_as_tuple },

  /* Generic stuff */
  { "__copy__", dec_copy, METH_NOARGS, NULL },
  { "__deepcopy__", dec_copy, METH_VARARGS, NULL },
  { "__format__", dec_format, METH_VARARGS, NULL },
  { "__reduce__", dec_reduce, METH_NOARGS, NULL },
  { "__round__", PyDec_Round, METH_VARARGS, NULL },
  { "__trunc__", PyDec_Trunc, METH_NOARGS, NULL },

  { NULL, NULL, 1 }
};

static PyTypeObject PyDec_Type =
{
	PyVarObject_HEAD_INIT(NULL, 0)
	"cdecimal.Decimal",                     /* tp_name */
	sizeof(PyDecObject),                    /* tp_basicsize */
	0,                                      /* tp_itemsize */
	(destructor) dec_dealloc,               /* tp_dealloc */
	0,                                      /* tp_print */
	(getattrfunc) 0,                        /* tp_getattr */
	(setattrfunc) 0,                        /* tp_setattr */
	0,                                      /* tp_reserved */
	(reprfunc) dec_repr,                    /* tp_repr */
	&dec_number_methods,                    /* tp_as_number */
	0,                                      /* tp_as_sequence */
	0,                                      /* tp_as_mapping */
	(hashfunc) dec_hash,                    /* tp_hash */
	0,                                      /* tp_call */
	(reprfunc) dec_str,                     /* tp_str */
	(getattrofunc) PyObject_GenericGetAttr, /* tp_getattro */
	(setattrofunc) 0,                       /* tp_setattro */
	(PyBufferProcs *) 0,                    /* tp_as_buffer */
	(Py_TPFLAGS_DEFAULT|
	 Py_TPFLAGS_BASETYPE),                  /* tp_flags */
	doc_decimal,                            /* tp_doc */
	0,                                      /* tp_traverse */
	0,                                      /* tp_clear */
	dec_richcompare,                        /* tp_richcompare */
	0,                                      /* tp_weaklistoffset */
	0,                                      /* tp_iter */
	0,                                      /* tp_iternext */
	dec_methods,                            /* tp_methods */
	0,                                      /* tp_members */
	0,                                      /* tp_getset */
	0,                                      /* tp_base */
	0,                                      /* tp_dict */
	0,                                      /* tp_descr_get */
	0,                                      /* tp_descr_set */
	0,                                      /* tp_dictoffset */
	0,                                      /* tp_init */
	0,                                      /* tp_alloc */
	dec_new,                                /* tp_new */
	PyObject_Del,                           /* tp_free */
};


/******************************************************************************/
/*                         Context Object, Part 2                             */
/******************************************************************************/


/************************************************************************/
/*     Macros for converting mpdecimal functions to Context methods     */
/************************************************************************/

/*
 * Operand is PyObject that must be convertible to a PyDecObject.
 * MPDFUNC uses a const context and does not raise.
 */
#define _DecCtx_BoolFunc(MPDFUNC) \
static PyObject *                                                        \
_DecCtx_##MPDFUNC(PyObject *self, PyObject *v)                           \
{                                                                        \
        PyObject *ret;                                                   \
        PyDecObject *a;                                                  \
        mpd_context_t *ctx;                                              \
                                                                         \
        ctx = CtxAddr(self);                                             \
        CONVERT_OP_SET(v, &a, ctx);                                      \
                                                                         \
        ret = MPDFUNC(a->dec, ctx) ? Dec_INCREF_TRUE : Dec_INCREF_FALSE; \
        Py_DECREF(a);                                                    \
        return ret;                                                      \
}

/*
 * Operand is PyObject that must be convertible to a PyDecObject.
 * MPDFUNC does NOT use a context and does not raise.
 */
#define _DecCtx_BoolFunc_NoCtx(MPDFUNC) \
static PyObject *                                                   \
_DecCtx_##MPDFUNC(PyObject *self, PyObject *v)                      \
{                                                                   \
        PyObject *ret;                                              \
        PyDecObject *a;                                             \
        mpd_context_t *ctx;                                         \
                                                                    \
        ctx = CtxAddr(self);                                        \
        CONVERT_OP_SET(v, &a, ctx);                                 \
                                                                    \
        ret = MPDFUNC(a->dec) ? Dec_INCREF_TRUE : Dec_INCREF_FALSE; \
        Py_DECREF(a);                                               \
        return ret;                                                 \
}

/*
 * Operand is PyObject that must be convertible to a PyDecObject.
 * MPDFUNC is a quiet function.
 */
#define _DecCtx_UnaryFunc(MPDFUNC) \
static PyObject *                                   \
_DecCtx_##MPDFUNC(PyObject *self, PyObject *v)      \
{                                                   \
        PyDecObject *result, *a;                    \
        mpd_context_t *ctx;                         \
        uint32_t status = 0;                        \
                                                    \
        ctx = CtxAddr(self);                        \
        CONVERT_OP_SET(v, &a, ctx);                 \
                                                    \
        if ((result = dec_alloc()) == NULL) {       \
                Py_DECREF(a);                       \
                return NULL;                        \
        }                                           \
                                                    \
        MPDFUNC(result->dec, a->dec, ctx, &status); \
        Py_DECREF(a);                               \
        if (dec_addstatus(ctx, status)) {           \
                Py_DECREF(result);                  \
                return NULL;                        \
        }                                           \
                                                    \
        return (PyObject *) result;                 \
}

/*
 * Operands are PyObjects that must be convertible to PyDecObjects.
 * MPDFUNC is a quiet function.
 */
#define _DecCtx_BinaryFunc(MPDFUNC) \
static PyObject *                                           \
_DecCtx_##MPDFUNC(PyObject *self, PyObject *args)           \
{                                                           \
        PyObject *v, *w;                                    \
        PyDecObject *a, *b;                                 \
        PyDecObject *result;                                \
        uint32_t status = 0;                                \
        mpd_context_t *ctx;                                 \
                                                            \
        if (!PyArg_ParseTuple(args, "OO", &v, &w)) {        \
                return NULL;                                \
        }                                                   \
                                                            \
        ctx = CtxAddr(self);                                \
        CONVERT_BINOP_SET(v, w, &a, &b, ctx);               \
                                                            \
        if ((result = dec_alloc()) == NULL) {               \
                Py_DECREF(a);                               \
                Py_DECREF(b);                               \
                return NULL;                                \
        }                                                   \
                                                            \
        MPDFUNC(result->dec, a->dec, b->dec, ctx, &status); \
        Py_DECREF(a);                                       \
        Py_DECREF(b);                                       \
        if (dec_addstatus(ctx, status)) {                   \
                Py_DECREF(result);                          \
                return NULL;                                \
        }                                                   \
                                                            \
        return (PyObject *) result;                         \
}

/*
 * Operands are Python Objects. Actual MPDFUNC does NOT take a context.
 * Uses context for conversion only.
 */
#define _DecCtx_BinaryFunc_NoCtx(MPDFUNC) \
static PyObject *                                    \
_DecCtx_##MPDFUNC(PyObject *self, PyObject *args)    \
{                                                    \
        PyObject *v, *w;                             \
        PyDecObject *a, *b;                          \
        PyDecObject *result;                         \
        mpd_context_t *ctx;                          \
                                                     \
        if (!PyArg_ParseTuple(args, "OO", &v, &w)) { \
                return NULL;                         \
        }                                            \
                                                     \
        ctx = CtxAddr(self);                         \
        CONVERT_BINOP_SET(v, w, &a, &b, ctx);        \
                                                     \
        if ((result = dec_alloc()) == NULL) {        \
                Py_DECREF(a);                        \
                Py_DECREF(b);                        \
                return NULL;                         \
        }                                            \
                                                     \
        MPDFUNC(result->dec, a->dec, b->dec);        \
        Py_DECREF(a);                                \
        Py_DECREF(b);                                \
                                                     \
        return (PyObject *) result;                  \
}

/* Operands are Python Objects. MPDFUNC is a quiet function. */
#define _DecCtx_TernaryFunc(MPDFUNC) \
static PyObject *                                                   \
_DecCtx_##MPDFUNC(PyObject *self, PyObject *args)                   \
{                                                                   \
        PyObject *v, *w, *x;                                        \
        PyDecObject *a, *b, *c;                                     \
        PyDecObject *result;                                        \
        uint32_t status = 0;                                        \
        mpd_context_t *ctx;                                         \
                                                                    \
        if (!PyArg_ParseTuple(args, "OOO", &v, &w, &x)) {           \
                return NULL;                                        \
        }                                                           \
                                                                    \
        ctx = CtxAddr(self);                                        \
        CONVERT_TERNOP_SET(v, w, x, &a, &b, &c, ctx);               \
                                                                    \
        if ((result = dec_alloc()) == NULL) {                       \
                Py_DECREF(a);                                       \
                Py_DECREF(b);                                       \
                Py_DECREF(c);                                       \
                return NULL;                                        \
        }                                                           \
                                                                    \
        MPDFUNC(result->dec, a->dec, b->dec, c->dec, ctx, &status); \
        Py_DECREF(a);                                               \
        Py_DECREF(b);                                               \
        Py_DECREF(c);                                               \
        if (dec_addstatus(ctx, status)) {                           \
                Py_DECREF(result);                                  \
                return NULL;                                        \
        }                                                           \
                                                                    \
        return (PyObject *) result;                                 \
}

static PyObject *
_DecCtx_copy_decimal(PyObject *self UNUSED, PyObject *v)
{
	PyDecObject *result;
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	CONVERT_OP_SET(v, &result, ctx);

	return (PyObject *) result;
}

/* Arithmetic operations */
_DecCtx_UnaryFunc(mpd_qabs)
_DecCtx_UnaryFunc(mpd_qexp)
_DecCtx_UnaryFunc(mpd_qinvroot)
_DecCtx_UnaryFunc(mpd_qln)
_DecCtx_UnaryFunc(mpd_qlog10)
_DecCtx_UnaryFunc(mpd_qminus)
_DecCtx_UnaryFunc(mpd_qnext_minus)
_DecCtx_UnaryFunc(mpd_qnext_plus)
_DecCtx_UnaryFunc(mpd_qplus)
_DecCtx_UnaryFunc(mpd_qreduce)
_DecCtx_UnaryFunc(mpd_qround_to_int)
_DecCtx_UnaryFunc(mpd_qround_to_intx)
_DecCtx_UnaryFunc(mpd_qsqrt)

_DecCtx_BinaryFunc(mpd_qadd)
_DecCtx_BinaryFunc(mpd_qcompare)
_DecCtx_BinaryFunc(mpd_qcompare_signal)
_DecCtx_BinaryFunc(mpd_qdiv)
_DecCtx_BinaryFunc(mpd_qdivint)
_DecCtx_BinaryFunc(mpd_qmax)
_DecCtx_BinaryFunc(mpd_qmax_mag)
_DecCtx_BinaryFunc(mpd_qmin)
_DecCtx_BinaryFunc(mpd_qmin_mag)
_DecCtx_BinaryFunc(mpd_qmul)
_DecCtx_BinaryFunc(mpd_qnext_toward)
_DecCtx_BinaryFunc(mpd_qpow)
_DecCtx_BinaryFunc(mpd_qquantize)
_DecCtx_BinaryFunc(mpd_qrem)
_DecCtx_BinaryFunc(mpd_qrem_near)
_DecCtx_BinaryFunc(mpd_qsub)

_DecCtx_TernaryFunc(mpd_qfma)
_DecCtx_TernaryFunc(mpd_qpowmod)

/* Miscellaneous */
_DecCtx_BoolFunc(mpd_isnormal)
_DecCtx_BoolFunc(mpd_issubnormal)
_DecCtx_BoolFunc_NoCtx(mpd_isfinite)
_DecCtx_BoolFunc_NoCtx(mpd_isinfinite)
_DecCtx_BoolFunc_NoCtx(mpd_isnan)
_DecCtx_BoolFunc_NoCtx(mpd_isqnan)
_DecCtx_BoolFunc_NoCtx(mpd_issigned)
_DecCtx_BoolFunc_NoCtx(mpd_issnan)
_DecCtx_BoolFunc_NoCtx(mpd_iszero)

static PyObject *
_DecCtx_mpd_qcopy_abs(PyObject *self, PyObject *v)
{
	PyDecObject *result, *a;
	uint32_t status = 0;
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	CONVERT_OP_SET(v, &a, ctx);

	if ((result = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qcopy_abs(result->dec, a->dec, &status);
	Py_DECREF(a);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *) result;
}

static PyObject *
_DecCtx_mpd_qcopy_negate(PyObject *self, PyObject *v)
{
	PyDecObject *result, *a;
	uint32_t status = 0;
	mpd_context_t *ctx;

	ctx = CtxAddr(self);
	CONVERT_OP_SET(v, &a, ctx);

	if ((result = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qcopy_negate(result->dec, a->dec, &status);
	Py_DECREF(a);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *) result;
}

_DecCtx_UnaryFunc(mpd_qinvert)
_DecCtx_UnaryFunc(mpd_qlogb)

_DecCtx_BinaryFunc_NoCtx(mpd_compare_total)
_DecCtx_BinaryFunc_NoCtx(mpd_compare_total_mag)

static PyObject *
_DecCtx_mpd_qcopy_sign(PyObject *self, PyObject *args)
{
	PyObject *v, *w;
	PyDecObject *a, *b;
	PyDecObject *result;
	uint32_t status = 0;
	mpd_context_t *ctx;

	if (!PyArg_ParseTuple(args, "OO", &v, &w)) {
		return NULL;
	}

	ctx = CtxAddr(self);
	CONVERT_BINOP_SET(v, w, &a, &b, ctx);

	if ((result = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qcopy_sign(result->dec, a->dec, b->dec, &status);
	Py_DECREF(a);
	Py_DECREF(b);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	return (PyObject *) result;
}

_DecCtx_BinaryFunc(mpd_qand)
_DecCtx_BinaryFunc(mpd_qor)
_DecCtx_BinaryFunc(mpd_qxor)

_DecCtx_BinaryFunc(mpd_qrotate)
_DecCtx_BinaryFunc(mpd_qscaleb)
_DecCtx_BinaryFunc(mpd_qshift)

static PyObject *
_DecCtx_iscanonical(PyObject *self UNUSED, PyObject *v)
{
	if (!PyDec_Check(v)) {
		PyErr_SetString(PyExc_TypeError,
		    "argument must be a Decimal.");
		return NULL;
	}

	return mpd_iscanonical(DecAddr(v)) ? Dec_INCREF_TRUE : Dec_INCREF_FALSE;
}

static PyObject *
_DecCtx_canonical(PyObject *self UNUSED, PyObject *v)
{
	if (!PyDec_Check(v)) {
		PyErr_SetString(PyExc_TypeError,
		    "argument must be a Decimal.");
		return NULL;
	}

	Py_INCREF(v);
	return v;
}

static PyObject *
_DecCtx_mpd_class(PyObject *self, PyObject *v)
{
	PyDecObject *a;
	mpd_context_t *ctx;
	const char *cp;

	ctx = CtxAddr(self);
	CONVERT_OP_SET(v, &a, ctx);

	cp = mpd_class(a->dec, ctx);
	Py_DECREF(a);

	return Py_BuildValue("s", cp);
}

static PyObject *
_DecCtx_mpd_qdivmod(PyObject *self, PyObject *args)
{
	PyObject *v, *w;
	PyDecObject *a, *b;
	PyDecObject *q, *r;
	uint32_t status = 0;
	mpd_context_t *ctx;

	if (!PyArg_ParseTuple(args, "OO", &v, &w)) {
		return NULL;
	}

	ctx = CtxAddr(self);
	CONVERT_BINOP_SET(v, w, &a, &b, ctx);

	if ((q = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}
	if ((r = dec_alloc()) == NULL) {
		Py_DECREF(a); /* GCOV_UNLIKELY */
		Py_DECREF(b); /* GCOV_UNLIKELY */
		Py_DECREF(q); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	mpd_qdivmod(q->dec, r->dec, a->dec, b->dec, ctx, &status);
	Py_DECREF(a);
	Py_DECREF(b);
	if (dec_addstatus(ctx, status)) {
		Py_DECREF(r);
		Py_DECREF(q);
		return NULL;
	}

	return Py_BuildValue("(NN)", q, r);
}

static PyObject *
_DecCtx_mpd_to_sci(PyObject *self, PyObject *v)
{
	PyObject *result;
	PyDecObject *a;
	mpd_context_t *ctx;
	char *s;

	ctx = CtxAddr(self);
	CONVERT_OP_SET(v, &a, ctx);

	s = mpd_to_sci(a->dec, CtxCaps(self));
	Py_DECREF(a);
	if (s == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	result = PyUnicode_FromString(s);
	mpd_free(s);

	return result;
}

static PyObject *
_DecCtx_mpd_to_eng(PyObject *self, PyObject *v)
{
	PyObject *result;
	PyDecObject *a;
	mpd_context_t *ctx;
	char *s;

	ctx = CtxAddr(self);
	CONVERT_OP_SET(v, &a, ctx);

	s = mpd_to_eng(a->dec, CtxCaps(self));
	Py_DECREF(a);
	if (s == NULL) {
		PyErr_NoMemory(); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	result = PyUnicode_FromString(s);
	mpd_free(s);

	return result;
}

static PyObject *
_DecCtx_mpd_radix(PyObject *self UNUSED, PyObject *dummy UNUSED)
{
	return Py_BuildValue("i", 10);
}

static PyObject *
_DecCtx_mpd_same_quantum(PyObject *self, PyObject *args)
{
	PyObject *v, *w;
	PyDecObject *a, *b;
	PyObject *result;
	mpd_context_t *ctx;

	if (!PyArg_ParseTuple(args, "OO", &v, &w)) {
		return NULL;
	}

	ctx = CtxAddr(self);
	CONVERT_BINOP_SET(v, w, &a, &b, ctx);

	result = mpd_same_quantum(a->dec, b->dec) ?
	             Dec_INCREF_TRUE : Dec_INCREF_FALSE;
	Py_DECREF(a);
	Py_DECREF(b);

	return result;
}


static PyMethodDef context_methods [] =
{
  /* Unary arithmetic functions */
  { "abs", _DecCtx_mpd_qabs, METH_O, doc_ctx_abs },
  { "exp", _DecCtx_mpd_qexp, METH_O, doc_ctx_exp },
  { "invroot", _DecCtx_mpd_qinvroot, METH_O, doc_ctx_invroot },
  { "ln", _DecCtx_mpd_qln, METH_O, doc_ctx_ln },
  { "log10", _DecCtx_mpd_qlog10, METH_O, doc_ctx_log10 },
  { "minus", _DecCtx_mpd_qminus, METH_O, doc_ctx_minus },
  { "next_minus", _DecCtx_mpd_qnext_minus, METH_O, doc_ctx_next_minus },
  { "next_plus", _DecCtx_mpd_qnext_plus, METH_O, doc_ctx_next_plus },
  { "normalize", _DecCtx_mpd_qreduce, METH_O, doc_ctx_normalize }, /* alias for reduce */
  { "plus", _DecCtx_mpd_qplus, METH_O, doc_ctx_plus },
  { "reduce", _DecCtx_mpd_qreduce, METH_O, doc_ctx_reduce },
  { "to_integral", _DecCtx_mpd_qround_to_int, METH_O, doc_ctx_to_integral },
  { "to_integral_exact", _DecCtx_mpd_qround_to_intx, METH_O, doc_ctx_to_integral_exact },
  { "to_integral_value", _DecCtx_mpd_qround_to_int, METH_O, doc_ctx_to_integral_value },
  { "sqrt", _DecCtx_mpd_qsqrt, METH_O, doc_ctx_sqrt },

  /* Binary arithmetic functions */
  { "add", _DecCtx_mpd_qadd, METH_VARARGS, doc_ctx_add },
  { "compare", _DecCtx_mpd_qcompare, METH_VARARGS, doc_ctx_compare },
  { "compare_signal", _DecCtx_mpd_qcompare_signal, METH_VARARGS, doc_ctx_compare_signal },
  { "div", _DecCtx_mpd_qdiv, METH_VARARGS, doc_ctx_div }, /* alias for divide */
  { "divide", _DecCtx_mpd_qdiv, METH_VARARGS, doc_ctx_divide },
  { "divide_int", _DecCtx_mpd_qdivint, METH_VARARGS, doc_ctx_divide_int },
  { "divint", _DecCtx_mpd_qdivint, METH_VARARGS, doc_ctx_divint }, /* alias for divide_int */
  { "divmod", _DecCtx_mpd_qdivmod, METH_VARARGS, doc_ctx_divmod },
  { "max", _DecCtx_mpd_qmax, METH_VARARGS, doc_ctx_max },
  { "max_mag", _DecCtx_mpd_qmax_mag, METH_VARARGS, doc_ctx_max_mag },
  { "min", _DecCtx_mpd_qmin, METH_VARARGS, doc_ctx_min },
  { "min_mag", _DecCtx_mpd_qmin_mag, METH_VARARGS, doc_ctx_min_mag },
  { "mul", _DecCtx_mpd_qmul, METH_VARARGS, doc_ctx_mul }, /* alias for multiply */
  { "multiply", _DecCtx_mpd_qmul, METH_VARARGS, doc_ctx_multiply },
  { "next_toward", _DecCtx_mpd_qnext_toward, METH_VARARGS, doc_ctx_next_toward },
  { "pow", _DecCtx_mpd_qpow, METH_VARARGS, doc_ctx_pow }, /* alias for power */
  { "power", _DecCtx_mpd_qpow, METH_VARARGS, doc_ctx_power },
  { "quantize", _DecCtx_mpd_qquantize, METH_VARARGS, doc_ctx_quantize },
  { "rem", _DecCtx_mpd_qrem, METH_VARARGS, doc_ctx_rem }, /* alias for remainder */
  { "remainder", _DecCtx_mpd_qrem, METH_VARARGS, doc_ctx_remainder },
  { "remainder_near", _DecCtx_mpd_qrem_near, METH_VARARGS, doc_ctx_remainder_near },
  { "sub", _DecCtx_mpd_qsub, METH_VARARGS, doc_ctx_sub }, /* alias for subtract */
  { "subtract", _DecCtx_mpd_qsub, METH_VARARGS, doc_ctx_subtract },

  /* Ternary arithmetic functions */
  { "fma", _DecCtx_mpd_qfma, METH_VARARGS, doc_ctx_fma },
  { "powmod", _DecCtx_mpd_qpowmod, METH_VARARGS, doc_ctx_powmod },

  /* No argument */
  { "Etiny", context_getetiny, METH_NOARGS, doc_ctx_Etiny },
  { "Etop", context_getetop, METH_NOARGS, doc_ctx_Etop },
  { "radix", _DecCtx_mpd_radix, METH_NOARGS, doc_ctx_radix },

  /* Boolean functions */
  { "is_canonical", _DecCtx_iscanonical, METH_O, doc_ctx_is_canonical },
  { "is_finite", _DecCtx_mpd_isfinite, METH_O, doc_ctx_is_finite },
  { "is_infinite", _DecCtx_mpd_isinfinite, METH_O, doc_ctx_is_infinite },
  { "is_nan", _DecCtx_mpd_isnan, METH_O, doc_ctx_is_nan },
  { "is_normal", _DecCtx_mpd_isnormal, METH_O, doc_ctx_is_normal },
  { "is_qnan", _DecCtx_mpd_isqnan, METH_O, doc_ctx_is_qnan },
  { "is_signed", _DecCtx_mpd_issigned, METH_O, doc_ctx_is_signed },
  { "is_snan", _DecCtx_mpd_issnan, METH_O, doc_ctx_is_snan },
  { "is_subnormal", _DecCtx_mpd_issubnormal, METH_O, doc_ctx_is_subnormal },
  { "is_zero", _DecCtx_mpd_iszero, METH_O, doc_ctx_is_zero },

  /* Functions with a single decimal argument */
  { "_apply", PyDecContext_Apply, METH_O, NULL }, /* alias for apply */
  { "apply", PyDecContext_Apply, METH_O, doc_ctx_apply },
  { "canonical", _DecCtx_canonical, METH_O, doc_ctx_canonical },
  { "copy_abs", _DecCtx_mpd_qcopy_abs, METH_O, doc_ctx_copy_abs },
  { "copy_decimal", _DecCtx_copy_decimal, METH_O, doc_ctx_copy_decimal },
  { "copy_negate", _DecCtx_mpd_qcopy_negate, METH_O, doc_ctx_copy_negate },
  { "logb", _DecCtx_mpd_qlogb, METH_O, doc_ctx_logb },
  { "logical_invert", _DecCtx_mpd_qinvert, METH_O, doc_ctx_logical_invert },
  { "number_class", _DecCtx_mpd_class, METH_O, doc_ctx_number_class },
  { "to_sci", _DecCtx_mpd_to_sci, METH_O, doc_ctx_to_sci }, /* alias for to_sci_string */
  { "to_sci_string", _DecCtx_mpd_to_sci, METH_O, doc_ctx_to_sci_string },
  { "to_eng", _DecCtx_mpd_to_eng, METH_O, doc_ctx_to_eng }, /* alias for to_eng_string */
  { "to_eng_string", _DecCtx_mpd_to_eng, METH_O, doc_ctx_to_eng_string },

  /* Functions with two decimal arguments */
  { "compare_total", _DecCtx_mpd_compare_total, METH_VARARGS, doc_ctx_compare_total },
  { "compare_total_mag", _DecCtx_mpd_compare_total_mag, METH_VARARGS, doc_ctx_compare_total_mag },
  { "copy_sign", _DecCtx_mpd_qcopy_sign, METH_VARARGS, doc_ctx_copy_sign },
  { "logical_and", _DecCtx_mpd_qand, METH_VARARGS, doc_ctx_logical_and },
  { "logical_or", _DecCtx_mpd_qor, METH_VARARGS, doc_ctx_logical_or },
  { "logical_xor", _DecCtx_mpd_qxor, METH_VARARGS, doc_ctx_logical_xor },
  { "rotate", _DecCtx_mpd_qrotate, METH_VARARGS, doc_ctx_rotate },
  { "same_quantum", _DecCtx_mpd_same_quantum, METH_VARARGS, doc_ctx_same_quantum },
  { "scaleb", _DecCtx_mpd_qscaleb, METH_VARARGS, doc_ctx_scaleb },
  { "shift", _DecCtx_mpd_qshift, METH_VARARGS, doc_ctx_shift },

  /* Set context values */
  { "setflags", PyDec_SetStatusFromList, METH_O, doc_ctx_setflags },
  { "settraps", PyDec_SetTrapsFromList, METH_O, doc_ctx_settraps },
  { "clear_flags", context_clear_flags, METH_NOARGS, doc_ctx_clear_flags },
  { "clear_traps", context_clear_traps, METH_NOARGS, doc_ctx_clear_traps },

  /* Unsafe set functions with no range checks */
  { "unsafe_setprec", context_unsafe_setprec, METH_O, NULL },
  { "unsafe_setemin", context_unsafe_setemin, METH_O, NULL },
  { "unsafe_setemax", context_unsafe_setemax, METH_O, NULL },

  /* Miscellaneous */
  { "__copy__", (PyCFunction)context_copy, METH_NOARGS, NULL },
  { "__reduce__", context_reduce, METH_NOARGS, NULL },
  { "copy", (PyCFunction)context_copy, METH_NOARGS, doc_ctx_copy },
  { "create_decimal", PyDecContext_CreateDecimal, METH_VARARGS, doc_ctx_create_decimal },
  { "create_decimal_from_float", PyDecContext_FromFloat, METH_O, doc_ctx_create_decimal_from_float },

  { NULL, NULL, 1 }
};

static PyTypeObject PyDecContext_Type =
{
	PyVarObject_HEAD_INIT(NULL, 0)
	"cdecimal.Context",                        /* tp_name */
	sizeof(PyDecContextObject),                /* tp_basicsize */
	0,                                         /* tp_itemsize */
	(destructor) context_dealloc,              /* tp_dealloc */
	0,                                         /* tp_print */
	(getattrfunc) 0,                           /* tp_getattr */
	(setattrfunc) 0,                           /* tp_setattr */
	0,                                         /* tp_reserved */
	(reprfunc) context_repr,                   /* tp_repr */
	0,                                         /* tp_as_number */
	0,                                         /* tp_as_sequence */
	0,                                         /* tp_as_mapping */
	(hashfunc) 0,                              /* tp_hash */
	0,                                         /* tp_call */
	(reprfunc) context_repr,                   /* tp_str */
	(getattrofunc) context_getattr,            /* tp_getattro */
	(setattrofunc) context_setattr,            /* tp_setattro */
	(PyBufferProcs *) 0,                       /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                        /* tp_flags */
	doc_context,                               /* tp_doc */
	0,                                         /* tp_traverse */
	0,                                         /* tp_clear */
	0,                                         /* tp_richcompare */
	0,                                         /* tp_weaklistoffset */
	0,                                         /* tp_iter */
	0,                                         /* tp_iternext */
	context_methods,                           /* tp_methods */
	0,                                         /* tp_members */
	context_getsets,                           /* tp_getset */
	0,                                         /* tp_base */
	0,                                         /* tp_dict */
	0,                                         /* tp_descr_get */
	0,                                         /* tp_descr_set */
	0,                                         /* tp_dictoffset */
	context_init,                              /* tp_init */
	0,                                         /* tp_alloc */
	context_new,                               /* tp_new */
	0                                          /* tp_free */
};


static PyMethodDef cdecimal_methods [] =
{
  { "getcontext", (PyCFunction)PyDec_GetCurrentContext, METH_NOARGS, doc_getcontext},
  { "setcontext", (PyCFunction)PyDec_SetCurrentContext, METH_O, doc_setcontext},
  { "localcontext", (PyCFunction)ctxmanager_new, METH_VARARGS, doc_localcontext},
  { "IEEEContext", (PyCFunction)ieee_context, METH_O, doc_ieee_context},
  { NULL, NULL, 1, NULL }
};

static struct PyModuleDef cdecimal_module = {
	PyModuleDef_HEAD_INIT,
	"cdecimal",
	doc_cdecimal,
	-1,
	cdecimal_methods,
	NULL,
	NULL,
	NULL,
	NULL
};


PyMODINIT_FUNC
PyInit_cdecimal(void)
{
	PyObject *m = NULL;
	PyObject *_numbers = NULL;
	PyObject *_Number = NULL;
	PyObject *obj, *ret, *s;
	PyObject *ctxobj;
	DecCondMap *cm;


	PyDec_Type.tp_base = &PyBaseObject_Type;
	PyDecContext_Type.tp_base = &PyBaseObject_Type;
	PyDecSignalDict_Type.tp_base = &PyDict_Type;
	PyDecContextManager_Type.tp_base = &PyBaseObject_Type;


	if (PyType_Ready(&PyDec_Type) < 0) {
		goto error; /* GCOV_UNLIKELY */
	}
	if (PyType_Ready(&PyDecContext_Type) < 0) {
		goto error; /* GCOV_UNLIKELY */
	}
	if (PyType_Ready(&PyDecSignalDict_Type) < 0) {
		goto error; /* GCOV_UNLIKELY */
	}
	if (PyType_Ready(&PyDecContextManager_Type) < 0) {
		goto error; /* GCOV_UNLIKELY */
	}


	if ((obj = PyUnicode_FromString("cdecimal")) == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	if (PyDict_SetItemString(PyDec_Type.tp_dict, "__module__", obj) < 0) {
		Py_DECREF(obj); /* GCOV_UNLIKELY */
		goto error; /* GCOV_UNLIKELY */
	}
	if (PyDict_SetItemString(PyDecContext_Type.tp_dict, "__module__", obj) < 0) {
		Py_DECREF(obj); /* GCOV_UNLIKELY */
		goto error; /* GCOV_UNLIKELY */
	}
	Py_DECREF(obj);


	if ((_numbers = PyImport_ImportModule("numbers")) == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	if ((_Number = PyObject_GetAttrString(_numbers, "Number")) == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	if ((obj = Py_BuildValue("O",  &PyDec_Type)) == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	if ((s = Py_BuildValue("s", "register")) == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	if ((ret = PyObject_CallMethodObjArgs(_Number, s, obj, NULL)) == NULL) {
		Py_DECREF(s); /* GCOV_UNLIKELY */
		Py_DECREF(obj); /* GCOV_UNLIKELY */
		goto error; /* GCOV_UNLIKELY */
	}
	Py_DECREF(s);
	Py_DECREF(obj);
	Py_DECREF(ret);

	if ((m = PyModule_Create(&cdecimal_module)) == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}

	mpd_traphandler = dec_traphandler;
	mpd_mallocfunc = PyMem_Malloc;
	mpd_reallocfunc = PyMem_Realloc;
	mpd_callocfunc = mpd_callocfunc_em;
	mpd_free = PyMem_Free;

	Py_INCREF(&PyDec_Type);
	PyModule_AddObject(m, "Decimal", (PyObject *)&PyDec_Type);

	Py_INCREF(&PyDecContext_Type);
	PyModule_AddObject(m, "Context", (PyObject *)&PyDecContext_Type);


	/* Top level Exception */
	DecimalException = PyErr_NewException("cdecimal.DecimalException",
	                                      PyExc_ArithmeticError, NULL);
	Py_INCREF(DecimalException);
	PyModule_AddObject(m, "DecimalException", DecimalException);

	/* Exceptions that correspond to IEEE signals */
	for (cm = signal_map; cm->name != NULL; cm++) {
		cm->dec_cond = PyErr_NewException((char *)cm->fqname,
		                                  DecimalException, NULL);
		Py_INCREF(cm->dec_cond);
		PyModule_AddObject(m, cm->name, cm->dec_cond);
	}

	/*
	 * Unfortunately, InvalidOperation is a signal that comprises
	 * several conditions, including InvalidOperation! Naming the
	 * signal IEEEInvalidOperation would prevent the confusion.
	 */
	cond_map[0].dec_cond = signal_map[0].dec_cond;

	/* Remaining exceptions, inherit from InvalidOperation */
	for (cm = cond_map+1; cm->name != NULL; cm++) {
		cm->dec_cond = PyErr_NewException((char *)cm->fqname,
		                                  signal_map[0].dec_cond , NULL);
		Py_INCREF(cm->dec_cond);
		PyModule_AddObject(m, cm->name, cm->dec_cond);
	}


	/* MPD_MINALLOC */
	mpd_setminalloc(4);

	/* Default context template */
	ctxobj = PyObject_CallObject((PyObject *)&PyDecContext_Type, NULL);
	if (ctxobj == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	default_context_template = ctxobj;
	Py_INCREF(ctxobj);
	PyModule_AddObject(m, "DefaultContext", ctxobj);

#ifdef WITHOUT_THREADS
	/* Module context */
	ctxobj = PyObject_CallObject((PyObject *)&PyDecContext_Type, NULL);
	if (ctxobj == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	module_context = ctxobj;
	PyModule_AddIntConstant(m, "HAVE_THREADS", 0);
#else
	tls_context_key = Py_BuildValue("s", "__CDECIMAL_CTX__");
	if (tls_context_key == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	PyModule_AddIntConstant(m, "HAVE_THREADS", 1);
#endif

	/* Basic context template */
	ctxobj = PyObject_CallObject((PyObject *)&PyDecContext_Type, NULL);
	if (ctxobj == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	init_basic_context(ctxobj);
	basic_context_template = ctxobj;
	Py_INCREF(ctxobj);
	PyModule_AddObject(m, "BasicContext", ctxobj);

	/* Extended context template */
	ctxobj = PyObject_CallObject((PyObject *)&PyDecContext_Type, NULL);
	if (ctxobj == NULL) {
		goto error; /* GCOV_UNLIKELY */
	}
	init_extended_context(ctxobj);
	extended_context_template = ctxobj;
	Py_INCREF(ctxobj);
	PyModule_AddObject(m, "ExtendedContext", ctxobj);


	/* Useful constants */
	PyModule_AddObject(m, "MAX_PREC", Py_BuildValue(CONV_mpd_ssize_t, MPD_MAX_PREC));
	PyModule_AddObject(m, "MAX_EMAX", Py_BuildValue(CONV_mpd_ssize_t, MPD_MAX_EMAX));
	PyModule_AddObject(m, "MIN_EMIN", Py_BuildValue(CONV_mpd_ssize_t, MPD_MIN_EMIN));
	PyModule_AddObject(m, "MIN_ETINY", Py_BuildValue(CONV_mpd_ssize_t, MPD_MIN_ETINY));

	PyModule_AddIntConstant(m, "DECIMAL32", MPD_DECIMAL32);
	PyModule_AddIntConstant(m, "DECIMAL64", MPD_DECIMAL64);
	PyModule_AddIntConstant(m, "DECIMAL128", MPD_DECIMAL128);
	PyModule_AddIntConstant(m, "IEEE_CONTEXT_MAX_BITS", MPD_IEEE_CONTEXT_MAX_BITS);

	PyModule_AddIntConstant(m, "ROUND_CEILING", MPD_ROUND_CEILING);
	PyModule_AddIntConstant(m, "ROUND_FLOOR", MPD_ROUND_FLOOR);
	PyModule_AddIntConstant(m, "ROUND_UP", MPD_ROUND_UP);
	PyModule_AddIntConstant(m, "ROUND_DOWN", MPD_ROUND_DOWN);
	PyModule_AddIntConstant(m, "ROUND_HALF_UP", MPD_ROUND_HALF_UP);
	PyModule_AddIntConstant(m, "ROUND_HALF_DOWN", MPD_ROUND_HALF_DOWN);
	PyModule_AddIntConstant(m, "ROUND_HALF_EVEN", MPD_ROUND_HALF_EVEN);
	PyModule_AddIntConstant(m, "ROUND_05UP", MPD_ROUND_05UP);
	PyModule_AddIntConstant(m, "ROUND_TRUNC", MPD_ROUND_TRUNC);

	/* Expose the C flags */
	PyModule_AddIntConstant(m, "DecClamped", MPD_Clamped);
	PyModule_AddIntConstant(m, "DecConversionSyntax", MPD_Conversion_syntax);
	PyModule_AddIntConstant(m, "DecDivisionByZero", MPD_Division_by_zero);
	PyModule_AddIntConstant(m, "DecDivisionImpossible", MPD_Division_impossible);
	PyModule_AddIntConstant(m, "DecDivisionUndefined", MPD_Division_undefined);
	PyModule_AddIntConstant(m, "DecFpuError", MPD_Fpu_error);
	PyModule_AddIntConstant(m, "DecInexact", MPD_Inexact);
	PyModule_AddIntConstant(m, "DecInvalidContext", MPD_Invalid_context);
	PyModule_AddIntConstant(m, "DecInvalidOperation", MPD_Invalid_operation);
	PyModule_AddIntConstant(m, "DecIEEEInvalidOperation", MPD_IEEE_Invalid_operation);
	PyModule_AddIntConstant(m, "DecMallocError", MPD_Malloc_error);
	PyModule_AddIntConstant(m, "DecFloatOperation", MPD_Float_operation);
	PyModule_AddIntConstant(m, "DecOverflow", MPD_Overflow);
	PyModule_AddIntConstant(m, "DecRounded", MPD_Rounded);
	PyModule_AddIntConstant(m, "DecSubnormal", MPD_Subnormal);
	PyModule_AddIntConstant(m, "DecUnderflow", MPD_Underflow);
	PyModule_AddIntConstant(m, "DecErrors", MPD_Errors);
	PyModule_AddIntConstant(m, "DecTraps", MPD_Traps);


	return m;

error: /* GCOV_UNLIKELY */
	if (_numbers) Py_DECREF(_numbers); /* GCOV_UNLIKELY */
	if (_Number) Py_DECREF(_Number); /* GCOV_UNLIKELY */
#ifdef WITHOUT_THREADS
	if (module_context) Py_DECREF(module_context); /* GCOV_UNLIKELY */
#else
	if (default_context_template) Py_DECREF(default_context_template); /* GCOV_UNLIKELY */
	if (tls_context_key) Py_DECREF(tls_context_key); /* GCOV_UNLIKELY */
#endif
	if (basic_context_template) Py_DECREF(basic_context_template); /* GCOV_UNLIKELY */
	if (extended_context_template) Py_DECREF(extended_context_template); /* GCOV_UNLIKELY */
	if (m) Py_DECREF(m); /* GCOV_UNLIKELY */

	return NULL; /* GCOV_UNLIKELY */
}


