/*
 * =====================================================================================
 *
 *       Filename:  wrapper.c
 *
 *    Description:  python module wrapper
 *
 *        Version:  1.0
 *        Created:  05/10/2013 10:12:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Reno
 *   Organization:  
 *
 * =====================================================================================
 */


#include <Python.h>

static PyObject *
symbolicate_wrapper(PyObject *self, PyObject *args)
{
    const char *arch, *executable, *addresses_str;
    int sts;
    char **addresses;

    int num_addresses;

    if (!PyArg_ParseTuple(args, "sss#", &arch, &executable, &addresses_str, &numofaddresses))
        return NULL;

    char *temp = addresses_str;
    while(*temp != '\0'){
        if (*temp == ' '){
            *temp = '\0';
        }
        temp ++;
        addresses_str[i] = ;
    }
    addresses = malloc(sizeof(char *) * num_addresses);
    int i = 0;
    while(i < num_addresses){
        addresses[i] = ;
        i++;
    }
    addresses[]
    sts = symbolicate(arch, executable, NULL);

    free(addresses);
    return Py_BuildValue("i", sts);
}
//static PyObject *
//symbolicate_wrapper(PyObject *self, PyObject *args)
//{
//    const char *arch, *executable, *addresses_str;
//    int sts;
//    char **addresses;
//
//    int num_addresses;
//
//    if (!PyArg_ParseTuple(args, "sss#", &arch, &executable, &addresses_str, &numofaddresses))
//        return NULL;
//
//    char *temp = addresses_str;
//    while(*temp != '\0'){
//        if (*temp == ' '){
//            *temp = '\0';
//        }
//        temp ++;
//        addresses_str[i] = ;
//    }
//    addresses = malloc(sizeof(char *) * num_addresses);
//    int i = 0;
//    while(i < num_addresses){
//        addresses[i] = ;
//        i++;
//    }
//    addresses[]
//    sts = symbolicate(arch, executable, NULL);
//
//    free(addresses);
//    return Py_BuildValue("i", sts);
//}

static PyMethodDef ATOSMethods[] = {
    {"symbolicate",  symbolicate_wrapper, METH_VARARGS,
    "binary address to symbol."},
    {NULL, NULL, 0, NULL}        /*  Sentinel */
};

PyMODINIT_FUNC initatos(void)
{
    (void) Py_InitModule("atos", ATOSMethods);
}

