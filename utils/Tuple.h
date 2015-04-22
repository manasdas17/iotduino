/*
 * tuple
 *
 *  Created on: 26.10.2014
 *      Author: helge
 */

#ifndef TUPLE_
#define TUPLE_

template<class type> class Tuple {
	private:
		type a;
		type b;

	public:
		Tuple() {
			
		}
		
		Tuple(type a, type b) {
			this->a = a;
			this->b = b;
		}

		type getA() {
			return a;
		}

		type getB() {
			return b;
		}
};



#endif /* TUPLE_ */
