#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

static const int defaultInitalAllocation = 10;
void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(elemSize > 0);
    assert(initialAllocation >= 0);
    if(initialAllocation == 0) {
        initialAllocation = defaultInitalAllocation;
    }
    v->elems = malloc(initialAllocation*elemSize);
    assert(v->elems != NULL);
    v->logLen = 0;
    v->allocLen = initialAllocation;
    v->initAlloc = initialAllocation;
    v->elemSize = elemSize;
    v->freeFn = freeFn;
}

void VectorDispose(vector *v)
{
    if (v->freeFn != NULL) {
        for (int i = 0; i < v->logLen; i++) {
            v->freeFn( (char*)v->elems + i*v->elemSize );
        }
    }
    free(v->elems);
}

int VectorLength(const vector *v)
{ 
    return v->logLen; 
}

void *VectorNth(const vector *v, int position)
{ 
    assert(position >= 0 && position < v->logLen);
    return (char *) v->elems + position * v->elemSize; 
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position < v->logLen);
    void *target = VectorNth(v, position);
    if (v->freeFn != NULL) {
        v->freeFn(target);
    }
    memcpy(target, elemAddr, v->elemSize);
}

static void VectorGrow(vector *v) 
{
    v->allocLen += v->initAlloc;
    v->elems = realloc(v->elems, v->allocLen * v->elemSize);
    assert(v->elems != NULL);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position <= v->logLen);
    if (v->logLen == v->allocLen) {
        VectorGrow(v);
    }
    void *target = (char *) v->elems + position * v->elemSize; // Dont use VectorNth here as insertion can be at v->logLen and it will give assertion error.
    int endSize = (v->logLen - position) * v->elemSize;
    if (endSize != 0) {
        memmove((char *)target + v->elemSize, target, endSize);
    }
    memcpy(target, elemAddr, v->elemSize);
    v->logLen++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    if (v->logLen == v->allocLen) {
        VectorGrow(v);
    }
    void *target = (char *) v->elems + v->logLen * v->elemSize;
    memcpy(target, elemAddr, v->elemSize);
    v->logLen++;
}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0 && position < v->logLen);
    void *target = VectorNth(v, position);
    if (v->freeFn != NULL) {
        v->freeFn(target);
    }
    int endSize = (v->logLen - position - 1) * v->elemSize;
    if (endSize != 0) { 
        memmove(target, (char *) target + v->elemSize, endSize);
    }
    v->logLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);
    qsort(v->elems, v->logLen, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);
    for (int i = 0; i < v->logLen; i++) {
        mapFn((char *) v->elems + i * v->elemSize, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
    assert(startIndex >= 0 && startIndex <= v->logLen);
    assert(searchFn != NULL);
    assert(key != NULL);

    void *found;
    size_t logLen = v->logLen;
    if (isSorted) {
        found = bsearch(key, v->elems, logLen, v->elemSize, searchFn);
    }
    else {
        found = lfind(key, v->elems, &logLen, v->elemSize, searchFn);
    }
    if (found == NULL) {
        return kNotFound;
    }
    return ((char *) found - (char *) v->elems) / v->elemSize;
} 
