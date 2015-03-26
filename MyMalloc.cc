//
// CS354: MyMalloc Project
//
// The current implementation gets memory from the OS
// every time memory is requested and never frees memory.
//
// You will implement the allocator as indicated in the handout.
// 
// Also you will need to add the necessary locking mechanisms to
// support multi-threaded programs.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

enum {
  ObjFree = 0,
  ObjAllocated = 1
};

// Header of an object. Used both when the object is allocated and freed
class ObjectHeader {
 public:
  int _flags;		      // flags == ObjFree or flags = ObjAllocated
  size_t _objectSize;         // Size of the object. Used both when allocated
			      // and freed.
};

class Allocator {
  // State of the allocator

  // Size of the heap
  size_t _heapSize;

  // True if heap has been initialized
  int _initialized;

  // Verbose mode
  int _verbose;

  // # malloc calls
  int _mallocCalls;

  // # free calls
  int _freeCalls;

  // # realloc calls
  int _reallocCalls;
  
  // # realloc calls
  int _callocCalls;
  
public:
  // This is the only instance of the allocator.
  static Allocator TheAllocator;

  //Initializes the heap
  void initialize();

  // Allocates an object 
  void * allocateObject( size_t size );

  // Frees an object
  void freeObject( void * ptr );

  // Returns the size of an object
  size_t objectSize( void * ptr );

  // At exit handler
  void atExitHandler();

  //Prints the heap size and other information about the allocator
  void print();

  // Gets memory from the OS
  void * getMemoryFromOS( size_t size );

  void increaseMallocCalls() { _mallocCalls++; }

  void increaseReallocCalls() { _reallocCalls++; }

  void increaseCallocCalls() { _callocCalls++; }

  void increaseFreeCalls() { _freeCalls++; }

};

Allocator Allocator::TheAllocator;

extern "C" void
atExitHandlerInC()
{
  Allocator::TheAllocator.atExitHandler();
}

void
Allocator::initialize()
{
  // Environment var VERBOSE prints stats at end and turns on debugging
  // Default is on
  _verbose = 1;
  const char * envverbose = getenv( "MALLOCVERBOSE" );
  if ( envverbose && !strcmp( envverbose, "NO") ) {
    _verbose = 0;
  }

  // In verbose mode register also printing statistics at exit
  atexit( atExitHandlerInC );

  _initialized = 1;
}

void *
Allocator::allocateObject( size_t size )
{
  // Simple implementation

  //Make sure that allocator is initialized
  if ( !_initialized ) {
    initialize();
  }

  // Add the ObjectHeader to the size and round the total size up to a
  // multiple of 8 bytes for alignment.
  size_t totalSize = (size + sizeof(ObjectHeader) + 7) & ~7;

  // You should get memory from the OS only if the memory in the free list could not
  // satisfy the request.

  // Simple allocator always gets memory from the OS.
  void * mem = getMemoryFromOS( totalSize );

  // Get a pointer to the object header
  ObjectHeader * o = (ObjectHeader *) mem;

  // Store the totalSize. We will need it in realloc() and in free()
  o->_objectSize = totalSize;

  // Set object as allocated
  o->_flags = ObjAllocated;

  // Return the pointer after the object header.
  return (void *) (o + 1);
}

void
Allocator::freeObject( void * ptr )
{
  // Here you will return the object to the free list sorted by address and you will coalesce it
  // if possible.

  // Simple allocator does nothing.
}

size_t
Allocator::objectSize( void * ptr )
{
  // Return the size of the object pointed by ptr. We assume that ptr is a valid obejct.
  ObjectHeader * o =
    (ObjectHeader *) ( (char *) ptr - sizeof(ObjectHeader) );

  // Substract the size of the header
  return o->_objectSize - sizeof(ObjectHeader);
}

void
Allocator::print()
{
  printf("\n-------------------\n");

  printf("HeapSize:\t%d bytes\n", _heapSize );
  printf("# mallocs:\t%d\n", _mallocCalls );
  printf("# reallocs:\t%d\n", _reallocCalls );
  printf("# callocs:\t%d\n", _callocCalls );
  printf("# frees:\t%d\n", _freeCalls );

  printf("\n-------------------\n");
}

void *
Allocator::getMemoryFromOS( size_t size )
{
  // Use sbrk() to get memory from OS
  _heapSize += size;
  
  return sbrk( size );
}

void
Allocator::atExitHandler()
{
  // Print statistics when exit
  if ( _verbose ) {
    print();
  }
}

//
// C interface
//

extern "C" void *
malloc(size_t size)
{
  Allocator::TheAllocator.increaseMallocCalls();
  
  return Allocator::TheAllocator.allocateObject( size );
}

extern "C" void
free(void *ptr)
{
  Allocator::TheAllocator.increaseFreeCalls();
  
  if ( ptr == 0 ) {
    // No object to free
    return;
  }
  
  Allocator::TheAllocator.freeObject( ptr );
}

extern "C" void *
realloc(void *ptr, size_t size)
{
  Allocator::TheAllocator.increaseReallocCalls();
    
  // Allocate new object
  void * newptr = Allocator::TheAllocator.allocateObject( size );

  // Copy old object only if ptr != 0
  if ( ptr != 0 ) {
    
    // copy only the minimum number of bytes
    size_t sizeToCopy =  Allocator::TheAllocator.objectSize( ptr );
    if ( sizeToCopy > size ) {
      sizeToCopy = size;
    }
    
    memcpy( newptr, ptr, sizeToCopy );

    //Free old object
    Allocator::TheAllocator.freeObject( ptr );
  }

  return newptr;
}

extern "C" void *
calloc(size_t nelem, size_t elsize)
{
  Allocator::TheAllocator.increaseCallocCalls();
    
  // calloc allocates and initializes
  size_t size = nelem * elsize;

  void * ptr = Allocator::TheAllocator.allocateObject( size );

  if ( ptr ) {
    // No error
    // Initialize chunk with 0s
    memset( ptr, 0, size );
  }

  return ptr;
}

extern "C" void 
checkHeap()
{
	// Verifies the heap consistency by iterating over all objects
	// in the free lists and checking that the next, previous pointers
	// size, and boundary tags make sense.
	// The checks are done by calling assert( expr ), where "expr"
	// is a condition that should be always true for an object.
	//
	// assert will print the file and line number and abort
	// if the expression "expr" is false.
	//
	// checkHeap() is required for your project and also it will be 
	// useful for debugging.
}

