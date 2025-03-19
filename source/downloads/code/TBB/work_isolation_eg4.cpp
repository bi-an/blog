#include "oneapi/tbb/task_arena.h"
#include "oneapi/tbb/parallel_for.h"
#include "oneapi/tbb/enumerable_thread_specific.h"
#include <cassert>


int main() {
    const int N1 = 1000, N2 = 1000;
    oneapi::tbb::enumerable_thread_specific<int> ets;
    oneapi::tbb::parallel_for( 0, N1, [&ets]( int i ) {
        // Set a thread specific value
        ets.local() = i;
        // Run the second parallel loop in an isolated region to prevent the current thread
        // from taking tasks related to the outer parallel loop.
        oneapi::tbb::this_task_arena::isolate( []{
            oneapi::tbb::parallel_for( 0, N2, []( int j ) { /* Some work */ } );
        } );
        assert( ets.local()==i ); // Valid assertion
    } );
    return 0;
}