#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0);
	assert(numBuckets > 0);
	assert(hashfn != NULL);
	assert(comparefn != NULL);
	h->classes = (vector *) malloc(numBuckets * sizeof(vector));
	assert(h->classes != NULL);
	h->numClasses = numBuckets;
	
	h->count = 0;
	h->hashFn = hashfn;
	h->cmpFn = comparefn;
	h->freeFn = freefn;

	for (int i = 0; i < numBuckets; i++) {
		VectorNew(&h->classes[i], elemSize, freefn, 0);
	}
}

void HashSetDispose(hashset *h)
{
	if (h->freeFn != NULL) {
		for (int i = 0; i < h->numClasses; i++) {
			VectorDispose(&h->classes[i]);
		}
	}

	free((void *) h->classes);
}

int HashSetCount(const hashset *h)
{ 
	return h->count;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != NULL);
	for (int i = 0; i < h->numClasses; i++) {
		VectorMap(&h->classes[i], mapfn, auxData);
	}	
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL);
	int hash = h->hashFn(elemAddr, h->numClasses);
	assert(hash >= 0 && hash < h->numClasses);

	int position = VectorSearch(&h->classes[hash], elemAddr, h->cmpFn, 0, false);
	if (position == -1) {
		// Append
		VectorAppend(&h->classes[hash], elemAddr);
	}
	else {
		// Replace
		VectorReplace(&h->classes[hash], elemAddr, position);
	}
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL);
	int hash = h->hashFn(elemAddr, h->numClasses);
	assert(hash >= 0 && hash < h->numClasses);

	int position = VectorSearch(&h->classes[hash], elemAddr, h->cmpFn, 0, false);
	if (position != -1) {
		return VectorNth(&h->classes[hash], position);
	}
	return NULL;
}
