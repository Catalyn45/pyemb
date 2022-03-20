#include <Python.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define error(format, ...) \
    printf("[ERROR] "format"\n" __VA_OPT__(,) __VA_ARGS__)

#define info(format, ...) \
    printf("[INFO]  "format"\n" __VA_OPT__(,) __VA_ARGS__)


static PyObject* echo(PyObject *self, PyObject *args) {
    const char* message;
    if(!PyArg_ParseTuple(args, "s", &message)) {
        error("Error at parsing arguments");
        return NULL;
    }

    info("got message: %s", message);

    return Py_BuildValue("s", message);
}

static PyMethodDef catalyn_methods[] = {
    {"echo", echo, METH_VARARGS, "Return string given"},
    {NULL, NULL, 0, NULL }
};

static struct PyModuleDef catalyn_module = {
    PyModuleDef_HEAD_INIT,
    "catalyn",
    "Custom functions written by catalyn",
    -1,
    catalyn_methods
};

PyMODINIT_FUNC PyInit_catalyn(void) {
    return PyModule_Create(&catalyn_module);
}

int main(int argc, char* argv[]) {
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);

    if(PyImport_AppendInittab("catalyn", PyInit_catalyn) == -1) {
        error("error at appending module");
        return -1;
    }

    Py_SetProgramName(program);
    Py_Initialize();

    if (argc < 2) {
        error("No python source file provided");
        goto free_prog;
    }

    const char* file_name = argv[1];
    FILE* file = fopen(file_name, "r");
    if (!file) {
        error("Error at opening file: %s", strerror(errno));
        goto free_prog;
    }

    char* path = realpath(file_name, NULL);
    if(!path) {
        error("error at getting real path: %s", strerror(errno));
        goto close_file;
    }

    info("full path: %s", path);

    if (PyRun_SimpleFile(file, path) == -1) { 
        error("Error at executing python file");
        goto free_path;
    }

free_path:
    free(path);
close_file:
    fclose(file);
free_prog:
    PyMem_RawFree(program);
    return 0;
}
