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
#include "main.h"

static PyObject *
symbolicate_wrapper(PyObject *self, PyObject *args)
{
    const char *arch, *executable;
    int result;

    int numofaddresses;
    PyObject* addresses_obj;

    if (!PyArg_ParseTuple(args, "ssO", &arch, &executable, &addresses_obj))
        return NULL;
            //|| !PyTuple_Check(addresses_obj))

    numofaddresses = PyTuple_Size(addresses_obj);
    //printf("numofaddresses: %d\n", numofaddresses);
    //unsigned int *addresses;
    //addresses = (int *) malloc(sizeof(unsigned int)*numofaddresses);
    char **addresses;
    addresses = malloc(sizeof(char *)*numofaddresses);
    int i = 0;
    PyObject *address_item;
    for (; i < numofaddresses; i++){
        address_item = PyTuple_GetItem(addresses_obj, i);
        if (PyString_Check(address_item)){
            addresses[i] = PyString_AsString(address_item);
            //printf("%s\n", addresses[i]);
        }else{
            //printf("Error: tuple contains a non-string value");
            exit(1);
        }
        //if (PyInt_Check(address_item)){
        //    addresses[i] = (unsigned int)PyInt_AsLong(address_item);
        //    printf("%d\n", addresses[i]);
        //}else{
        //    printf("Error: tuple contains a non-int value");
        //    exit(1);
        //}
    }

    result = symbolicate(arch, executable, addresses, numofaddresses);

    free(addresses);
    //printf("end\n");
    return Py_BuildValue("i", result);
}
//static PyObject *
//symbolicate_wrapper(PyObject *self, PyObject *args)
//{
//    const char *arch, *executable, *addresses_str;
//    int sts;
//    char **addresses;
//
//    int numofaddresses;
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
//    addresses = malloc(sizeof(char *) * numofaddresses);
//    int i = 0;
//    while(i < numofaddresses){
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

