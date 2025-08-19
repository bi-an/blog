oneapi::tbb::enumerable_thread_specific<int> ets;
oneapi::tbb::task_arena nested;
oneapi::tbb::parallel_for( 0, N1, [&]( int i ) {
    // Set a thread specific value
    ets.local() = i;
    nested.execute( []{
        // Run the inner parallel_for in a separate arena to prevent the thread
        // from taking tasks of the outer parallel_for.
        oneapi::tbb::parallel_for( 0, N2, []( int j ) { /* Some work */ } );
    } );
    assert( ets.local()==i ); // Valid assertion
} );