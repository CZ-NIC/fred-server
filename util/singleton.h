#ifndef SINGLETON_H_
#define SINGLETON_H_

template<class _Tp> class Singleton {
public:
	static _Tp* instance_ptr() {
		if (m_instance == 0) {
			m_instance = new _Tp();
		}
		return m_instance;
	}
	static _Tp& instance_ref() {
		if (m_instance == 0) {
			m_instance = new _Tp();
		}
		return *m_instance;
	}
	void destroy() {
		if (m_instance) {
			delete m_instance;
			m_instance = 0;
		}
	}

private:
	Singleton() {
	}
	Singleton(const Singleton<_Tp> &_s) {
	}
	~Singleton() {
	}
	Singleton<_Tp>& operator=(Singleton<_Tp> const &_s) {
		return *this;
	}

	static _Tp* m_instance;
};

template<class _Tp> _Tp* Singleton<_Tp>::m_instance = 0;

#endif /*SINGLETON_H_*/
