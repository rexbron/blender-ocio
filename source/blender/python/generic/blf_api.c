/**
 * $Id$
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Campbell Barton
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <Python.h>
#include "blf_api.h"

#include "../../blenfont/BLF_api.h"

static char py_blf_position_doc[] =
".. function:: position(fontid, x, y, z)\n"
"\n"
"   Set the position for drawing text.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg x: X axis position to draw the text.\n"
"   :type x: float\n"
"   :arg y: Y axis position to draw the text.\n"
"   :type y: float\n"
"   :arg z: Z axis position to draw the text.\n"
"   :type x: float\n";

static PyObject *py_blf_position(PyObject *self, PyObject *args)
{
	int fontid;
	float x, y, z;

	if (!PyArg_ParseTuple(args, "ifff:BLF.position", &fontid, &x, &y, &z))
		return NULL;

	BLF_position(fontid, x, y, z);

	Py_RETURN_NONE;
}


static char py_blf_size_doc[] =
".. function:: size(fontid, size, dpi)\n"
"\n"
"   Set the size and dpi for drawing text.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg size: Point size of the font.\n"
"   :type size: int\n"
"   :arg dpi: dots per inch value to use for drawing.\n"
"   :type dpi: int\n";

static PyObject *py_blf_size(PyObject *self, PyObject *args)
{
	int fontid, size, dpi;

	if (!PyArg_ParseTuple(args, "iii:BLF.size", &fontid, &size, &dpi))
		return NULL;

	BLF_size(fontid, size, dpi);

	Py_RETURN_NONE;
}


static char py_blf_aspect_doc[] =
".. function:: aspect(fontid, aspect)\n"
"\n"
"   Set the aspect for drawing text.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg aspect: The aspect ratio for text drawing to use.\n"
"   :type aspect: float\n";

static PyObject *py_blf_aspect(PyObject *self, PyObject *args)
{
	float aspect;
	int fontid;

	if (!PyArg_ParseTuple(args, "if:BLF.aspect", &fontid, &aspect))
		return NULL;

	BLF_aspect(fontid, aspect);

	Py_RETURN_NONE;
}


static char py_blf_blur_doc[] =
".. function:: blur(fontid, radius)\n"
"\n"
"   Set the blur radius for drawing text.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg radius: The radius for blurring text (in pixels).\n"
"   :type radius: int\n";

static PyObject *py_blf_blur(PyObject *self, PyObject *args)
{
	int blur, fontid;

	if (!PyArg_ParseTuple(args, "ii:BLF.blur", &fontid, &blur))
		return NULL;

	BLF_blur(fontid, blur);

	Py_RETURN_NONE;
}


static char py_blf_draw_doc[] =
".. function:: draw(fontid, text)\n"
"\n"
"   Draw text in the current context.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg text: the text to draw.\n"
"   :type text: string\n";

static PyObject *py_blf_draw(PyObject *self, PyObject *args)
{
	char *text;
	int fontid;

	if (!PyArg_ParseTuple(args, "is:BLF.draw", &fontid, &text))
		return NULL;

	BLF_draw(fontid, text);

	Py_RETURN_NONE;
}

static char py_blf_dimensions_doc[] =
".. function:: dimensions(fontid, text)\n"
"\n"
"   Return the width and hight of the text.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg text: the text to draw.\n"
"   :type text: string\n"
"   :return: the width and height of the text.\n"
"   :rtype: tuple of 2 floats\n";

static PyObject *py_blf_dimensions(PyObject *self, PyObject *args)
{
	char *text;
	float r_width, r_height;
	PyObject *ret;
	int fontid;

	if (!PyArg_ParseTuple(args, "is:BLF.dimensions", &fontid, &text))
		return NULL;

	BLF_width_and_height(fontid, text, &r_width, &r_height);

	ret= PyTuple_New(2);
	PyTuple_SET_ITEM(ret, 0, PyFloat_FromDouble(r_width));
	PyTuple_SET_ITEM(ret, 1, PyFloat_FromDouble(r_height));
	return ret;
}

static char py_blf_clipping_doc[] =
".. function:: clipping(fontid, xmin, ymin, xmax, ymax)\n"
"\n"
"   Set the clipping, enable/disable using CLIPPING.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg xmin: Clip the drawing area by these bounds.\n"
"   :type xmin: float\n"
"   :arg ymin: Clip the drawing area by these bounds.\n"
"   :type ymin: float\n"
"   :arg xmax: Clip the drawing area by these bounds.\n"
"   :type xmax: float\n"
"   :arg ymax: Clip the drawing area by these bounds.\n"
"   :type ymax: float\n";

static PyObject *py_blf_clipping(PyObject *self, PyObject *args)
{
	float xmin, ymin, xmax, ymax;
	int fontid;

	if (!PyArg_ParseTuple(args, "iffff:BLF.clipping", &fontid, &xmin, &ymin, &xmax, &ymax))
		return NULL;

	BLF_clipping(fontid, xmin, ymin, xmax, ymax);

	Py_RETURN_NONE;
}

static char py_blf_disable_doc[] =
".. function:: disable(fontid, option)\n"
"\n"
"   Disable option.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg option: One of ROTATION, CLIPPING, SHADOW or KERNING_DEFAULT.\n"
"   :type option: int\n";

static PyObject *py_blf_disable(PyObject *self, PyObject *args)
{
	int option, fontid;

	if (!PyArg_ParseTuple(args, "ii:BLF.disable", &fontid, &option))
		return NULL;

	BLF_disable(fontid, option);

	Py_RETURN_NONE;
}

static char py_blf_enable_doc[] =
".. function:: enable(fontid, option)\n"
"\n"
"   Enable option.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg option: One of ROTATION, CLIPPING, SHADOW or KERNING_DEFAULT.\n"
"   :type option: int\n";

static PyObject *py_blf_enable(PyObject *self, PyObject *args)
{
	int option, fontid;

	if (!PyArg_ParseTuple(args, "ii:BLF.enable", &fontid, &option))
		return NULL;

	BLF_enable(fontid, option);

	Py_RETURN_NONE;
}

static char py_blf_rotation_doc[] =
".. function:: rotation(fontid, angle)\n"
"\n"
"   Set the text rotation angle, enable/disable using ROTATION.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg angle: The angle for text drawing to use.\n"
"   :type aspect: float\n";

static PyObject *py_blf_rotation(PyObject *self, PyObject *args)
{
	float angle;
	int fontid;

	if (!PyArg_ParseTuple(args, "if:BLF.rotation", &fontid, &angle))
		return NULL;
		
	BLF_rotation(fontid, angle);

	Py_RETURN_NONE;
}

static char py_blf_shadow_doc[] =
".. function:: shadow(fontid, level, r, g, b, a)\n"
"\n"
"   Shadow options, enable/disable using SHADOW .\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg level: The blur level, can be 3, 5 or 0.\n"
"   :type level: int\n"
"   :arg r: Shadow color (red channel 0.0 - 1.0).\n"
"   :type r: float\n"
"   :arg g: Shadow color (green channel 0.0 - 1.0).\n"
"   :type g: float\n"
"   :arg b: Shadow color (blue channel 0.0 - 1.0).\n"
"   :type b: float\n"
"   :arg a: Shadow color (alpha channel 0.0 - 1.0).\n"
"   :type a: float\n";

static PyObject *py_blf_shadow(PyObject *self, PyObject *args)
{
	int level, fontid;
	float r, g, b, a;

	if (!PyArg_ParseTuple(args, "iiffff:BLF.shadow", &fontid, &level, &r, &g, &b, &a))
		return NULL;

	if (level != 0 && level != 3 && level != 5) {
		PyErr_SetString(PyExc_TypeError, "blf.shadow expected arg to be in (0, 3, 5)");
		return NULL;
	}

	BLF_shadow(fontid, level, r, g, b, a);

	Py_RETURN_NONE;
}

static char py_blf_shadow_offset_doc[] =
".. function:: shadow_offset(fontid, x, y)\n"
"\n"
"   Set the offset for shadow text.\n"
"\n"
"   :arg fontid: The id of the typeface as returned by :func:`blf.load`, for default font use 0.\n"
"   :type fontid: int\n"
"   :arg x: Vertical shadow offset value in pixels.\n"
"   :type x: float\n"
"   :arg y: Horizontal shadow offset value in pixels.\n"
"   :type y: float\n";

static PyObject *py_blf_shadow_offset(PyObject *self, PyObject *args)
{
	int x, y, fontid;

	if (!PyArg_ParseTuple(args, "iii:BLF.shadow_offset", &fontid, &x, &y))
		return NULL;

	BLF_shadow_offset(fontid, x, y);

	Py_RETURN_NONE;
}

static char py_blf_load_doc[] =
".. function:: load(filename)\n"
"\n"
"   Load a new font.\n"
"\n"
"   :arg filename: the filename of the font.\n"
"   :type filename: string\n"
"   :return: the new font's fontid or -1 if there was an error.\n"
"   :rtype: integer\n";

static PyObject *py_blf_load(PyObject *self, PyObject *args)
{
	char* filename;

	if (!PyArg_ParseTuple(args, "s:blf.load", &filename))
		return NULL;

	return PyLong_FromLong(BLF_load(filename));
}

/*----------------------------MODULE INIT-------------------------*/
struct PyMethodDef BLF_methods[] = {
	{"aspect", (PyCFunction) py_blf_aspect, METH_VARARGS, py_blf_aspect_doc},
	{"blur", (PyCFunction) py_blf_blur, METH_VARARGS, py_blf_blur_doc},
	{"clipping", (PyCFunction) py_blf_clipping, METH_VARARGS, py_blf_clipping_doc},
	{"disable", (PyCFunction) py_blf_disable, METH_VARARGS, py_blf_disable_doc},
	{"dimensions", (PyCFunction) py_blf_dimensions, METH_VARARGS, py_blf_dimensions_doc},
	{"draw", (PyCFunction) py_blf_draw, METH_VARARGS, py_blf_draw_doc},
	{"enable", (PyCFunction) py_blf_enable, METH_VARARGS, py_blf_enable_doc},
	{"position", (PyCFunction)py_blf_position, METH_VARARGS, py_blf_position_doc},
	{"rotation", (PyCFunction) py_blf_rotation, METH_VARARGS, py_blf_rotation_doc},
	{"shadow", (PyCFunction) py_blf_shadow, METH_VARARGS, py_blf_shadow_doc},
	{"shadow_offset", (PyCFunction) py_blf_shadow_offset, METH_VARARGS, py_blf_shadow_offset_doc},
	{"size", (PyCFunction) py_blf_size, METH_VARARGS, py_blf_size_doc},
	{"load", (PyCFunction) py_blf_load, METH_VARARGS, py_blf_load_doc},
	{NULL, NULL, 0, NULL}
};

static char BLF_doc[] =
"This module provides access to blenders text drawing functions.\n";

static struct PyModuleDef BLF_module_def = {
	PyModuleDef_HEAD_INIT,
	"blf",  /* m_name */
	BLF_doc,  /* m_doc */
	0,  /* m_size */
	BLF_methods,  /* m_methods */
	0,  /* m_reload */
	0,  /* m_traverse */
	0,  /* m_clear */
	0,  /* m_free */
};

PyObject *BLF_Init(void)
{
	PyObject *submodule;

	submodule = PyModule_Create(&BLF_module_def);
	PyDict_SetItemString(PySys_GetObject("modules"), BLF_module_def.m_name, submodule);

	PyModule_AddIntConstant(submodule, "ROTATION", BLF_ROTATION);
	PyModule_AddIntConstant(submodule, "CLIPPING", BLF_CLIPPING);
	PyModule_AddIntConstant(submodule, "SHADOW", BLF_SHADOW);
	PyModule_AddIntConstant(submodule, "KERNING_DEFAULT", BLF_KERNING_DEFAULT);

	return (submodule);
}