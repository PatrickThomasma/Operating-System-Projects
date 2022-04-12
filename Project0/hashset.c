#include "hashset.h"  /* the .h file does NOT reside in /usr/local/include/ADTs */
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include "ADTs/iterator.h"
/* any other includes needed by your code */
#define UNUSED __attribute__((unused))
#define DEFAULT_CAPACITY 16
#define MAX_CAPACITY 134217728L
#define DEFAULT_LOAD_FACTOR 0.75
#define TRIGGER 100

typedef struct mentry {
	void *key;
	void *value;
}MEntry;

typedef struct node {
	struct node *next;
	MEntry entry;
}Node;

typedef struct s_data {
	long (*hashFxn) (void *, long N);
	int (*cmpFxn)(void *, void *);
	int index;
	long size;
	long capacity;
	long changes;
	double load;
	double loadFactor;
	double increment;
	Node **buckets;
	void (*freeValue)(void *v);
    /* definitions of the data members of self */
} SData;

/*
 * important - remove UNUSED attributed in signatures when you flesh out the
 * methods
 */

/*
#define SHIFT 31L
long hashFxn(void *key, long N) {
	uint64_t k = (uint64_t)key;
	k ^= (k >> 30);
	k *= 0xbf58476d1ce435b9;
	k ^= (k >> 27);
	k *= 0x94d049bb133111eb;
	k ^= (k >> 31);
	return (k & 0x7fffffffffffffff) % N;
}

int cmpFxn(void *v1, void *v2) {
	return strcmp((char *) v1, (char *)v2);
}
*/
/*
long hashFxn(void *key, long N) {
	char *sp = (char *)key;
	unsigned long ans;
	for ( ; *sp != '\0'; sp++)
		ans = SHIFT * ans + (unsigned long)*sp;
	return (long) (ans % N);

}
*/

static void purge(SData *sd, void (*freeValue)(void *v)) {
	long i;
	for (i = 0L; i < sd->capacity; i++) {
		Node *p, *q;
		p = sd->buckets[i];
		while (p != NULL) {
			if (freeValue != NULL)
				(*freeValue)((p->entry).key);
			q = p->next;
			free(p);
			p = q;
		}
		sd->buckets[i] = NULL;
	}
}
		

static void s_destroy(const Set *s) {
    /* implement the destroy() method */
	SData *sd = (SData *)s->self;
	purge(sd,sd->freeValue);
	free(sd->buckets);
	free(sd);
	free((void *)s);
}

static void s_clear(const Set *s) {
    /* implement the clear() method */
    SData *sd = (SData *)s->self;
    purge(sd, sd->freeValue);
    sd->size = 0;
    sd->load = 0.0;
    sd->changes = 0;
}

static void resize (SData *sd) {
	int N;
	Node *p, *q, **array;
	long i, j;
	N = 2 * sd->capacity;
	if (N > MAX_CAPACITY)
		N = MAX_CAPACITY;
	if (N == sd->capacity)
		return;
	array = (Node **)malloc(N * sizeof(Node *));
	if (array == NULL)
		return;
	for (j = 0; j < N; j++)
		array[j] = NULL;
	for (i = 0; i < sd->capacity; i++) {
		for (p = sd->buckets[i]; p != NULL; p = q) {
			q = p->next;
			j = sd->hashFxn((p->entry).value, N);
			p->next = array[j];
			array[j] = p;
		}
	}
	free (sd->buckets);
	sd->buckets = array;
	sd->capacity = N;
	sd->load /= 2.0;
	sd->changes = 0;
	sd->increment = 1.0 / (double)N;
}

static int insert(SData *sd, void *value, long i) {
	Node *p = (Node *)malloc(sizeof(Node));
	int ans = 0;

	if (p != NULL) {
		(p->entry).value = value;
		(p->entry).key = value;
		p->next = sd->buckets[i];
		sd->buckets[i] = p;
		sd->index++;
		sd->size++;
		sd->load += sd->increment;
		sd->changes++;
		ans = 1;
		}
	return ans;
}

static Node *find(SData *sd, void *key, long *bucket) {
	
	long i = sd->hashFxn( key, sd->capacity);
	int ans = 0;

	Node *p;

	*bucket = i;

	for (p = sd->buckets[i]; p != NULL; p = p->next) {
		if (sd->cmpFxn((p->entry).key, key) == 0) {
			break;
		}
	}


	return p;
}


static bool s_add(const Set *s, void *member) {
	SData *sd = (SData *) s->self;
	long i;
	Node *p;
	int ans = 0;
	if (sd->changes > TRIGGER) {
		sd->changes = 0;
		if (sd->load > sd->loadFactor)
			resize(sd);
	}

	p  = find(sd, member ,&i);
	if (p == NULL) {

		ans = insert(sd, member,i);
	}

	return ans;
}


static bool s_contains(const Set *s, void *member) {
    /* implement the contains() method */
    SData *sd = (SData *)s->self;
    long bucket;


    
    return (find(sd,member, &bucket) != NULL);
}

static bool s_isEmpty(const Set *s) {
    SData *sd = (SData *) s->self;

    return (sd->size == 0L);
}

static bool s_remove(const Set *s, void *member) {
    /* implement the remove() method */
    SData *sd = (SData *)s->self;
    long i;
    Node *entry;
    int ans = 0;
    entry = find(sd, member ,&i);
    if (entry != NULL) {
	    Node *p, *c;
	    for (p = NULL, c = sd->buckets[i]; c != entry; p = c, c = c->next)
		    ;
	    if (p == NULL)
		    sd ->buckets[i] = entry->next;
	    else
		    p->next = entry->next;
	    sd->size--;
	    sd->load -= sd->increment;
	    sd->changes++;
	    if (sd->freeValue != NULL)
		    sd->freeValue((entry->entry).value);
	    free(entry);
	    ans = 1;
    }
    return ans;
}
static long s_size(const Set *s) {
    /* implement the size() method */
    SData *sd = (SData *) s->self;
    return sd->size;
}

static void **keys (SData *sd) {
	void **tmp = NULL;
	if (sd->size > 0L) {
		size_t nbytes = sd->size * sizeof(void *);
		tmp = (void **)malloc(nbytes);
		if (tmp != NULL) {
			long i, n = 0L;
			for (i = 0L; i < sd->capacity; i++) {
				Node *p = sd->buckets[i];
				while (p != NULL) {
					tmp[n++] = (p->entry).key;
					p = p->next;
				}
			}
		}

	}
	return tmp;
}


static void **s_toArray(const Set *s, long *len) {
    /* implement the toArray() method */
    SData *sd = (SData *)s->self;
    void **tmp = keys(sd);

    if (tmp != NULL)
	    *len = sd->size;
    return tmp;
}

static MEntry **entries (SData *sd) {
        MEntry **tmp = NULL;
        if (sd->size > 0L) {
                size_t nbytes = sd->size * sizeof(MEntry *);
                tmp = (MEntry **)malloc(nbytes);
                if (tmp != NULL) {
                        long i, n = 0L;
                        for (i = 0L; i < sd->capacity; i++) {
                                Node *p = sd->buckets[i];
                                while (p != NULL) {
                                        tmp[n++] =  p->entry.value;
                                        p = p->next;
                                }
                        }
                }

        }
        return tmp;
}

static MEntry **m_entryArray(const Set *s, long *len) {
	SData *sd = (SData *)s->self;
	MEntry **tmp = entries(sd);
	if (tmp != NULL)
		*len = sd->size;
	return tmp;
}

static const Iterator *s_itCreate(const Set *s) {
    /* implement the itCreate() method */
    
    SData *sd = (SData *)s->self;
    const Iterator *it = NULL;
    void **tmp = (void **)entries(sd);
    if (tmp != NULL) {
	    it = Iterator_create(sd->size, tmp);
	    if (it == NULL) 
		    free(tmp);
	    
    }
    return it;
}

static const Set *s_create(const Set *s);

static Set template = {
    NULL, s_destroy, s_clear, s_add, s_contains, s_isEmpty, s_remove,
    s_size, s_toArray, s_itCreate
};

const Set *newHashSet(void (*freeValue)(void*), int (*cmpFxn)(void*, void*),
                   long capacity, double loadFactor,
                   long (*hashFxn)(void *m, long N)
                  ) {

	Set *s = (Set *)malloc(sizeof(Set));
	long N;
	double lf;
	Node **array;
	long i;

	if (s != NULL) {
		SData *sd = (SData *) malloc(sizeof(SData));

		if (sd != NULL) {
			N = ((capacity > 0) ? capacity : DEFAULT_CAPACITY);
			if (N > MAX_CAPACITY)
				N = MAX_CAPACITY;
			lf = ((loadFactor > 0.000001) ? loadFactor : DEFAULT_LOAD_FACTOR);
			array = (Node **)malloc(N * sizeof(Node *));
			if (array != NULL) {
				sd->capacity = N;
				sd->loadFactor = lf;
				sd->size = 0L;
				sd->index = 0;
				sd->load = 0.0;
				sd->changes = 0L;
				sd->increment = 1.0 / (double)N;
				sd->hashFxn = hashFxn;
				sd->cmpFxn = cmpFxn;
				sd->freeValue = freeValue;
				sd->buckets = array;
				for (i = 0; i < N; i++)
					array[i] = NULL;
				*s = template;
				s->self = sd;
			} else {
				free(sd);
				free(s);
				s = NULL;
			}
		} else {
			free(s);
			s = NULL;
		}
	}
	return s;
}

static const Set *s_create(const Set *s) {
	SData *sd = (SData *) s->self;
	return newHashSet(sd->freeValue, sd->cmpFxn,sd->capacity,sd->loadFactor,sd->hashFxn);
}

const Set *HashSet(void (*freeValue)(void*), int (*cmpFxn)(void*, void*),
                   long capacity, double loadFactor,
                   long (*hashFxn)(void *m, long N)
                  ) {

	return newHashSet(freeValue, cmpFxn, capacity, loadFactor, hashFxn);
}

