/*
This source code file is part of thread_mpi.  
Written by Sander Pronk, Erik Lindahl, and possibly others. 

Copyright (c) 2009, Sander Pronk, Erik Lindahl.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1) Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2) Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3) Neither the name of the copyright holders nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY US ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WE BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

If you want to redistribute modifications, please consider that
scientific software is very special. Version control is crucial -
bugs must be traceable. We will be happy to consider code for
inclusion in the official distribution, but derived work should not
be called official thread_mpi. Details are found in the README & COPYING
files.
*/

/* IBM xlC compiler on AIX */
#include <sys/atomic_op.h>


#define tMPI_Atomic_memory_barrier()


typedef struct tMPI_Atomic
{
    volatile int value;  /*!< Volatile, to avoid compiler aliasing */
}
tMPI_Atomic_t;


typedef struct tMPI_Atomic_ptr
{
    void* volatile* value;  /*!< Volatile, to avoid compiler aliasing */
}
tMPI_Atomic_ptr_t;


typedef struct tMPI_Spinlock
{
    volatile unsigned int lock;  /*!< Volatile, to avoid compiler aliasing */
}
tMPI_Spinlock_t;


static inline int tMPI_Atomic_cas(tMPI_Atomic_t *a, int oldval, int newval)
{
    int t;
    
    if(__check_lock((atomic_p)&a->value, oldval, newval))
    {
        /* Not successful - value had changed in memory. Reload value. */
        t = a->value;
    }
    else
    {
        /* replacement suceeded */
        t = oldval;
    }
    return t;        
}


static inline void* tMPI_Atomic_ptr_cas(tMPI_Atomic_ptr_t *a, void* oldval,
                                        void* newval)
{
    void *t;
    
    if(__check_lock((atomic_p)&a->value, oldval, newval))
    {
        /* Not successful - value had changed in memory. Reload value. */
        t = a->value;
    }
    else
    {
        /* replacement suceeded */
        t = oldval;
    }
    return t;        
}




static inline void tMPI_Atomic_add_return(tMPI_Atomic_t *a, int i)
{
    int oldval,newval;    
    
    do
    {
        oldval = tMPI_Atomic_get(a);
        newval = oldval + i;
    }
    while(__check_lock((atomic_p)&a->value, oldval, newval));

    return newval;
}



static inline void tMPI_Atomic_fetch_add(tMPI_Atomic_t *a, int i)
{
    int oldval,newval;    
    
    do
    {
        oldval = tMPI_Atomic_get(a);
        newval = oldval + i;
    }
    while(__check_lock((atomic_p)&a->value, oldval, newval));
    
    return oldval;
}


static inline void tMPI_Spinlock_init(tMPI_Spinlock_t *x)
{
    __clear_lock((atomic_p)x,0);
}


static inline void tMPI_Spinlock_lock(tMPI_Spinlock_t *x)
{
    do
    {
        ;
    }
    while(__check_lock((atomic_p)x, 0, 1));
}


static inline void tMPI_Spinlock_trylock(tMPI_Spinlock_t *x)
{
    /* Return 0 if we got the lock */
    return (__check_lock((atomic_p)x, 0, 1) != 0)
}


static inline void tMPI_Spinlock_unlock(tMPI_Spinlock_t *x)
{
    __clear_lock((atomic_p)x,0);
}


static inline void tMPI_Spinlock_islocked(const tMPI_Spinlock_t *x)
{
    return (*((atomic_p)x) != 0);
}


static inline void tMPI_Spinlock_wait(tMPI_Spinlock_t *x)
{
    while(spin_islocked(x)) { ; } 
}


