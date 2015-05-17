/*
 * singleton.h
 *
 * Created: 06.10.2014 22:11:35
 *  Author: helge
 */ 


#ifndef SINGLETON_H_
#define SINGLETON_H_

template <typename C> class Singleton {
	public:
		static C* instance () {
			if (!_instance)
				_instance = new C ();
			return _instance;
		}
		virtual ~Singleton () {
			_instance = 0;
		}
	private:
		static C* _instance;
	protected:
		Singleton () {}
};
template <typename C> C* Singleton <C>::_instance = 0;
#endif /* SINGLETON_H_ */