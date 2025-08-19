oneapi::tbb::enumerable_thread_specific<int> ets;
oneapi::tbb::parallel_for( 0, N1, [&ets]( int i ) {
    // Set a thread specific value
    ets.local() = i;
    oneapi::tbb::parallel_for( 0, N2, []( int j ) { /* Some work */ } );
    // While executing the above parallel_for, the thread might have run iterations
    // of the outer parallel_for, and so might have changed the thread specific value.
    assert( ets.local()==i ); // The assertion may fail!
} );