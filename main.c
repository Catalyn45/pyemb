#include <Python.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define error(format, ...) \
    printf("[ERROR] "format"\n" __VA_OPT__(,) __VA_ARGS__)

#define info(format, ...) \
    printf("[INFO]  "format"\n" __VA_OPT__(,) __VA_ARGS__)

#define str_errno strerror(errno)

static PyObject* echo(PyObject *self, PyObject *args) {
    const char* message;
    if(!PyArg_ParseTuple(args, "s", &message)) {
        error("Error at parsing arguments");
        return NULL;
    }

    info("got message: %s", message);

    return Py_BuildValue("s", message);
}

static PyObject* compute(PyObject *self, PyObject *args) {
    unsigned int max_nr;
    if(!PyArg_ParseTuple(args, "I", &max_nr)) {
        error("Error at parsing arguments");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    info("start computing");
    for (int i = 0; i < max_nr; i++) {
        volatile int res = (i + 2012) % 555;
    }
    info("done computing");
    Py_END_ALLOW_THREADS

    Py_RETURN_TRUE;
}

static PyMethodDef catalyn_methods[] = {
    {"echo", echo, METH_VARARGS, "Return string given"},
    {"compute", compute, METH_VARARGS, "Compute big string"},
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


static char* read_file(const char* file_name) {
    FILE* f = fopen(file_name, "r");
    if(!f) {
        error("error opening file: %s", str_errno);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    char *content = malloc(fsize + 1);
    if(!content) {
        error("error alocating memory: %s", str_errno);
        goto free_file;
    }

    fread(content, fsize, 1, f);
    fclose(f);
    content[fsize] = '\0';
    return content;

free_memory:
    free(content);

free_file:
    fclose(f);
    return NULL;

}

int main(int argc, char* argv[]) {
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);

    if(PyImport_AppendInittab("catalyn", PyInit_catalyn) == -1) {
        error("error at appending module");
        return 0;
    }

    Py_SetProgramName(program);
    Py_Initialize();

    char* module_content = read_file("modules/test_module.py");
    if(!module_content) {
        error("fail to get module content");
        goto free_prog;
    }

    PyObject *module = Py_CompileString(module_content, "test_module.py", Py_file_input);
    free(module_content);
    if(!module) {
        error("error compiling module");
        goto free_prog;
    }

    PyObject *module_obj = PyImport_ExecCodeModule("test_module", module);
    if(!module_obj) {
        error("error importing module");
        goto free_prog;
    }

    if (argc < 2) {
        error("No python source file provided");
        goto free_prog;
    }

    const char* file_name = argv[1];
    FILE* file = fopen(file_name, "r");
    if (!file) {
        error("Error at opening file: %s", str_errno);
        goto free_prog;
    }

    char* path = realpath(file_name, NULL);
    if(!path) {
        error("error at getting real path: %s", str_errno);
        goto close_file;
    }

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
