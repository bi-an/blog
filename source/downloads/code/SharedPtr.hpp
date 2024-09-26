      template<_Lock_policy _Lp = __default_lock_policy>
    class _Sp_counted_base
    : public _Mutex_base<_Lp>
    {
      void
      _M_add_ref_copy()
      { __gnu_cxx::__atomic_add_dispatch(&_M_use_count, 1); }

      void
      _M_release() noexcept
      {
        // Be race-detector-friendly.  For more info see bits/c++config.
        _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(&_M_use_count);
	if (__gnu_cxx::__exchange_and_add_dispatch(&_M_use_count, -1) == 1)
	  {
            _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(&_M_use_count);
	    _M_dispose();
	    // There must be a memory barrier between dispose() and destroy()
	    // to ensure that the effects of dispose() are observed in the
	    // thread that runs destroy().
	    // See http://gcc.gnu.org/ml/libstdc++/2005-11/msg00136.html
	    if (_Mutex_base<_Lp>::_S_need_barriers)
	      {
		__atomic_thread_fence (__ATOMIC_ACQ_REL);
	      }
    };

  template<_Lock_policy _Lp>
    class __shared_count
    {
      _Sp_counted_base<_Lp>*  _M_pi; // 引用计数的指针

      __shared_count(const __shared_count& __r) noexcept
      : _M_pi(__r._M_pi)
      {
	if (_M_pi != nullptr)
	  _M_pi->_M_add_ref_copy(); // TODO: 有没有可能此时的 _M_pi 所引对象已经被释放了呢
      }

      ~__shared_count() noexcept
      {
	if (_M_pi != nullptr)
	  _M_pi->_M_release(); // 没有直接 delete _M_pi，
                           // 而是使用 _M_pi 的成员函数来释放它。
      }

      __shared_count&
      operator=(const __shared_count& __r) noexcept
      {
	_Sp_counted_base<_Lp>* __tmp = __r._M_pi;
	if (__tmp != _M_pi)
	  {
	    if (__tmp != nullptr)
	      __tmp->_M_add_ref_copy();
	    if (_M_pi != nullptr)
	      _M_pi->_M_release(); // 没有直接 delete _M_pi，
                               // 而是使用 _M_pi 的成员函数来释放它。
	    _M_pi = __tmp;
	  }
	return *this;
      }
    };
