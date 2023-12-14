#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#include <numpy/npy_common.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define DIM 1536
// #define num_vectors 121248

float dot_product(const float *vec1, const float *vec2)
{

    float score = 0.0;

#pragma clang loop unroll(enable) interleave(enable) vectorize(enable)
    for (int i = 0; i < DIM; i++)
    {
        score += vec1[i] * vec2[i];
    }
    return score;
}

void search(int dim, int num_vectors, int top_k, float *search_vector, char *filename, int **o_best_indices, float **o_best_scores)
{

    const char *filepath = filename;
    int fd = open(filepath, O_RDWR);
    if (fd < 0)
    {
        printf("\n\"%s \" could not open\n", filepath);
        exit(1);
    }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if (err < 0)
    {
        printf("\n\"%s \" could not open\n", filepath);
        exit(2);
    }

    char *ptr = mmap(NULL, statbuf.st_size,
                     PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        printf("Mapping Failed\n");
    }
    close(fd);

    const float *fptr = (float *)ptr;

    float *vec1 = malloc(DIM * sizeof(float));
    float *best_scores = malloc(top_k * sizeof(float));
    int *best_indices = malloc(top_k * sizeof(int));

    for (int i = 0; i < top_k; i++)
    {
        best_scores[i] = 1e9;
        best_indices[i] = -1;
    }

    struct timespec time1, time2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

    for (int i = 0; i < DIM; i++)
    {
        vec1[i] = fptr[i];
    }

    for (int i = 0; i < top_k; i++)
    {
        best_scores[i] = 1e9;
        best_indices[i] = -1;
    }

    for (int index = 0; index < num_vectors; index++)
    {

        float score = dot_product(fptr, vec1);
        score = 1 - score;

        for (int i = 0; i < top_k; i++)
        {
            if (score < best_scores[i])
            {
                for (int j = top_k - 1; j > i; j--)
                {
                    best_scores[j] = best_scores[j - 1];
                    best_indices[j] = best_indices[j - 1];
                }
                best_scores[i] = score;
                best_indices[i] = index;
                break;
            }
        }
        fptr += DIM;
        // printf("Score => %f\n",score);
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);

    printf("Best Scores =>\n");
    for (int i = 0; i < top_k; i++)
    {
        printf("%f ", best_scores[i]);
    }
    printf("\nBest Indices =>\n");
    for (int i = 0; i < top_k; i++)
    {
        printf("%d ", best_indices[i]);
    }
    double elapsed = (time2.tv_sec - time1.tv_sec) * 1e3 + (time2.tv_nsec - time1.tv_nsec) / 1e6;
    printf("\nTime taken: %f ms\n", elapsed);

    /*
        ssize_t n = write(1,ptr,statbuf.st_size);
        if(n != statbuf.st_size){
            printf("Write failed\n");
        }
        */
    printf("Done\n");
    /*
        // Reverse the file contents
        for(size_t i=0; i < statbuf.st_size/2; i++){

            int j = statbuf.st_size - i - 1;
            int t = ptr[i];
            ptr[i] = ptr[j];
            ptr[j] = t;
        }

        printf("\n Reading Reversed file =>\n");
        n = write(1,ptr,statbuf.st_size);
        if(n != statbuf.st_size){
             printf("Write failed\n");
        }

    */
    err = munmap(ptr, statbuf.st_size);

    if (err != 0)
    {
        printf("UnMapping Failed\n");
    }
    *o_best_indices = best_indices;
    *o_best_scores = best_scores;
}

typedef struct
{
    PyObject_HEAD
        /* Type-specific fields go here. */

        int dim;
    char *filename;
    int num_vectors;
    char *ptr;
    size_t map_size;

} SparVecSucherObject;

static PyObject *SparVecSucher_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SparVecSucherObject *self;
    self = (SparVecSucherObject *)type->tp_alloc(type, 0);

    if (self != NULL)
    {
        self->dim = 0;
        self->filename = NULL;
        self->num_vectors = 0;
        self->ptr = NULL;
    }
    return (PyObject *)self;
}

static void SparVecSucher_dealloc(SparVecSucherObject *self)
{
    int err = munmap(self->ptr, self->map_size);
    
    

    Py_TYPE(self)->tp_free((PyObject *)self);
    if (err != 0) {
        // set python error
        PyErr_SetString(PyExc_ValueError, "UnMapping Failed");
    }
}

static int SparVecSucher_init(SparVecSucherObject *self, PyObject *args, PyObject *kwds)
{
    char *filename;
    int num_vectors;

    if (!PyArg_ParseTuple(args, "si", &filename, &num_vectors))
    {
        return -1;
    }
    self->filename = filename;
    self->num_vectors = num_vectors;
    self->dim = 1536;

    const char *filepath = filename;
    int fd = open(filepath, O_RDWR);
    if (fd < 0)
    {
        PyErr_SetString(PyExc_ValueError, "File not found");
        return -1;
    }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if (err < 0)
    {
        PyErr_SetString(PyExc_ValueError, "File not found, fstat failed");
        return -1;
    }
    char *ptr = mmap(NULL, statbuf.st_size,
                     PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        PyErr_SetString(PyExc_ValueError, "mmap failed");
        return -1;
    }
    self->ptr = ptr;
    self->map_size = statbuf.st_size;
    close(fd);
    return 0;
}

static PyObject * SparVecSucher_search(SparVecSucherObject *self, PyObject *args, PyObject *kwds)
{
    npy_intp *shape;
    float *data;
    int top_k;
    PyObject *obj = NULL; // Temporary object for parsing


    printf("Searching...\n");

    if (!PyArg_ParseTuple(args, "Oi", &obj, &top_k))
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    if (!PyArray_Check(obj))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a NumPy array.");
        return NULL;
    }
    PyArrayObject *search_vector = (PyArrayObject *)obj;

    search_vector = (PyArrayObject *)PyArray_FROM_OTF((PyObject *)search_vector,
                                                      NPY_FLOAT32,
                                                      NPY_ARRAY_IN_ARRAY);
    if (search_vector == NULL)
    {
        return NULL;
    }

    data = (double *)PyArray_DATA(search_vector);
    shape = PyArray_SHAPE(search_vector);
    int n = shape[0]; // Assuming a 1D array for simplicity
    if (n != DIM)
    {
        PyErr_SetString(PyExc_ValueError, "Array must be of size 1536");
        Py_DECREF(search_vector);
        return NULL;
    }


    printf("Parsed\n");

    if (!PyArray_Check(search_vector))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a NumPy array.");
        return NULL;
    }
    printf("Serching top %d\n", top_k);

    float *best_scores = malloc(top_k * sizeof(float));
    int *best_indices = malloc(top_k * sizeof(int));
    printf("best scores allocated\n");
    int dim = self->dim;
    int num_vectors = self->num_vectors;
    const float *fptr = (float *)self->ptr;

    float *vec1 = data;

    for (int i = 0; i < top_k; i++)
    {
        best_scores[i] = 1e9;
        best_indices[i] = -1;
    }

    printf("Init done\n");

    struct timespec time1, time2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

    for (int i = 0; i < dim; i++)
    {
        vec1[i] = data[i];
    }

    for (int i = 0; i < top_k; i++)
    {
        best_scores[i] = 1e9;
        best_indices[i] = -1;
    }

    for (int index = 0; index < num_vectors; index++)
    {
        float score = dot_product(fptr, vec1);
        score = 1 - score;

        for (int i = 0; i < top_k; i++)
        {
            if (score < best_scores[i])
            {
                for (int j = top_k - 1; j > i; j--)
                {
                    best_scores[j] = best_scores[j - 1];
                    best_indices[j] = best_indices[j - 1];
                }
                best_scores[i] = score;
                best_indices[i] = index;
                break;
            }
        }
        fptr += dim;
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);

    printf("Best Scores =>\n");
    for (int i = 0; i < top_k; i++)
    {
        printf("%f ", best_scores[i]);
    }
    printf("\nBest Indices =>\n");
    for (int i = 0; i < top_k; i++)
    {
        printf("%d ", best_indices[i]);
    }
    double elapsed = (time2.tv_sec - time1.tv_sec) * 1e3 + (time2.tv_nsec - time1.tv_nsec) / 1e6;
    printf("\nTime taken: %f ms\n", elapsed);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef SparVecSucher_methods[] = {
    {"search", (PyCFunction)SparVecSucher_search, METH_VARARGS,
     "Search the vector"},

    {NULL} /* Sentinel */
};

static PyTypeObject SparVecSucherType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "c_module.SparVecSucher",
    .tp_doc = "Memory mapped file for vector search",
    .tp_basicsize = sizeof(SparVecSucherObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = SparVecSucher_new,
    .tp_dealloc = (destructor)SparVecSucher_dealloc,
    .tp_methods = SparVecSucher_methods,
    .tp_init = (initproc)SparVecSucher_init,
};

static PyObject *py_search(PyObject *self, PyObject *args)
{
    // void search(int dim, int num_vectors, int top_k, float *search_vector, char * filename, int **o_best_indices, float ** o_best_scores)

    int result;
    char *filename;
    int num_vectors;
    int top_k;

    npy_intp *shape;
    double *data;
    PyObject *obj = NULL; // Temporary object for parsing

    // Parse all arguments at once
    if (!PyArg_ParseTuple(args, "Oisi", &obj, &top_k, &filename, &num_vectors))
    {
        return NULL;
    }

    // Ensure the object is a NumPy array

    if (!PyArray_Check(obj))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a NumPy array.");
        return NULL;
    }
    PyArrayObject *search_vector = (PyArrayObject *)obj;

    search_vector = (PyArrayObject *)PyArray_FROM_OTF((PyObject *)search_vector,
                                                      NPY_DOUBLE,
                                                      NPY_ARRAY_IN_ARRAY);
    if (search_vector == NULL)
    {
        return NULL;
    }

    data = (double *)PyArray_DATA(search_vector);
    shape = PyArray_SHAPE(search_vector);
    int n = shape[0]; // Assuming a 1D array for simplicity
    if (n != DIM)
    {
        PyErr_SetString(PyExc_ValueError, "Array must be of size 1536");
        Py_DECREF(search_vector);
        return NULL;
    }

    float *best_scores;
    int *best_indices;

    search(DIM, num_vectors, top_k, data, filename, &best_indices, &best_scores);

    Py_DECREF(search_vector);
    Py_INCREF(Py_None);
    return Py_None;
}

// Define the methods of the module
static PyMethodDef c_module_methods[] = {
    {"search", py_search, METH_VARARGS, "search"},
    {NULL, NULL, 0, NULL} // Sentinel
};

// Define the module
static struct PyModuleDef c_module_definition = {
    PyModuleDef_HEAD_INIT,
    "c_module",
    "The module that contains the c code for the vector search.",
    -1,
    c_module_methods};

// Initialization function for the module
PyMODINIT_FUNC PyInit_c_module(void)
{

    PyObject *m;
    if (PyType_Ready(&SparVecSucherType) < 0)
        return NULL;

    m = PyModule_Create(&c_module_definition);
    if (m == NULL)
        return NULL;

    Py_INCREF(&SparVecSucherType);
    if (PyModule_AddObject(m, "SparVecSucher", (PyObject *)&SparVecSucherType) < 0)
    {
        Py_DECREF(&SparVecSucherType);
        Py_DECREF(m);
        return NULL;
    }

    // Initialize NumPy API
    import_array();

    return m;
}
