// The first parallel loop.
oneapi::tbb::parallel_for( 0, N1, []( int i ) {
    // The second parallel loop.
    oneapi::tbb::parallel_for( 0, N2, []( int j ) { /* Some work */ } );
} );