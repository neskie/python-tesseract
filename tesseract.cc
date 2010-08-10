#include <Python.h>
#include <tesseract/baseapi.h>
#include <tesseract/img.h>
#include <tesseract/imgs.h>
#include <iulib/io_tiff.h>
#include <structmember.h>

//We'll use module methods to alter the object.
//In the future I'll create a new tesseractobject
//with object methods.
#ifdef _TIFFIO_
void read_tiff_image(TIFF* tif, IMAGE* image);
#endif
typedef struct {
	PyObject_HEAD
	TessBaseAPI *api;
	PyObject *lang;
	PyObject *filename;
	PyObject *text;
	int mode;
	/* Type-specific fields go here. */
} Tesseract;

static void
Tesseract_dealloc(Tesseract* self)
{
	Py_XDECREF(self->api);
	Py_XDECREF(self->lang);
	Py_XDECREF(self->filename);
	Py_XDECREF(self->text);
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Tesseract_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Tesseract *self;

	self = (Tesseract *)type->tp_alloc(type, 0);
	if (self != NULL) {
	self->api = new TessBaseAPI();
        if (self->api == NULL)
	{
		Py_DECREF(self);
		return NULL;
          }
	self->lang = PyString_FromString("");
	if (self->lang == NULL)
	{
		Py_DECREF(self);
		return NULL;
	}
	self->filename = PyString_FromString("");
	if (self->filename == NULL)
	{
		Py_DECREF(self);
		return NULL;
	}
	self->text = PyString_FromString("");
	if (self->text == NULL)
	{
		Py_DECREF(self);
		return NULL;
	}
	self->mode = 0;
    }

    return (PyObject *)self;
}

static
PyObject *
get_text(Tesseract *self, PyObject *args)
{
	IMAGE image;
	IMAGE *imageptr = &image;
	char * inputname = PyString_AsString(self->filename);
	TIFF* tif = TIFFOpen(inputname, "r");
	if (tif) {
		read_tiff_image(tif, &image);
		TIFFClose(tif);
	} else
	return  PyString_FromString("Could Not Open Tiff file.");
	int bytes_per_line = check_legal_image_size(imageptr->get_xsize(),
                                              imageptr->get_ysize(),
                                              imageptr->get_bpp());

	char * text;
	if (self->mode == 0) {
	text = (self->api)->TesseractRect(imageptr->get_buffer(),
		imageptr->get_bpp()/8, bytes_per_line, 0, 0,
		imageptr->get_xsize(), imageptr->get_ysize());
	}
	else if (self->mode == 1){
	text = (self->api)->TesseractRectBoxes(imageptr->get_buffer(),
		imageptr->get_bpp()/8, bytes_per_line, 0, 0,
		imageptr->get_xsize(), imageptr->get_ysize(), imageptr->get_ysize());
	}
	return  PyString_FromString(text);

}

static int
Tesseract_init(Tesseract *self, PyObject *args, PyObject *kwds)
{
	TessBaseAPI *api = new TessBaseAPI();
	PyObject *lang=NULL, *filename=NULL, *stmp;
	static char *kwlist[] = {"lang","filename","mode", NULL};

	if (! PyArg_ParseTupleAndKeywords(args,kwds, "|OOi",kwlist,
						&lang,&filename,&self->mode))
		return -1;

	if (lang) {
		stmp = self->lang;
		Py_INCREF(lang);
		self->lang = lang;
		Py_XDECREF(stmp);
	}
	if (filename) {
		stmp = self->filename;
		Py_INCREF(filename);
		self->filename = filename;
		Py_XDECREF(stmp);
	}
	if (api) {
		Py_INCREF(api);
		self->api = api;
		(self->api)->InitWithLanguage("tesseract",NULL, PyString_AsString(lang), NULL, false, 0, NULL);
	}
	return 0;
}

static PyMemberDef Tesseract_members[] = {
	{"lang", T_OBJECT_EX, offsetof(Tesseract, lang),0,
         "Tesseract Language"},
	{"filename", T_OBJECT_EX, offsetof(Tesseract, filename),0,
         "Tesseract Filename"},
	{"mode", T_INT, offsetof(Tesseract, mode),0,
         "Tesseract mode"},
	{NULL} /*Sentinel */
};

static PyMethodDef Tesseract_methods[] = {
	{"get_text", (PyCFunction)get_text, METH_NOARGS}, 
	{NULL}  /* Sentinel */
};

static PyTypeObject tesseract_TesseractType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"tesseract.Tesseract",             /*tp_name*/
	sizeof(Tesseract), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)Tesseract_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	"Tesseract objects",           /* tp_doc */
	0,		               /* tp_traverse */
	0,		               /* tp_clear */
	0,		               /* tp_richcompare */
	0,		               /* tp_weaklistoffset */
	0,		               /* tp_iter */
	0,		               /* tp_iternext */
	Tesseract_methods,             /* tp_methods */
	Tesseract_members,             /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)Tesseract_init,      /* tp_init */
	0,                         /* tp_alloc */
	Tesseract_new,                 /* tp_new */

};

PyMODINIT_FUNC inittesseract(void)
{
	PyObject* m;

	tesseract_TesseractType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&tesseract_TesseractType) < 0)
	return;

	m = Py_InitModule3("tesseract", Tesseract_methods,
		       "Module that provides an interface to tesseract.");

	Py_INCREF(&tesseract_TesseractType);
	PyModule_AddObject(m, "Tesseract", (PyObject *)&tesseract_TesseractType);

}
