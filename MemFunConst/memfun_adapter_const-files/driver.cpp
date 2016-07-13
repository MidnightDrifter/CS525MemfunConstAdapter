#include <iostream>
#include <iomanip> //setw
#include <vector>
#include <list>
#include <algorithm>
#include <cstdio> //sscanf

#include "memfun.adapter.h"

class A {
	int i;
	public:
		A(int _i) : i(_i) {}
		const int& Get() const { return i; }
        int Power( unsigned exp ) {
            int res = 1;
            for ( unsigned p=0; p<exp; ++p ) res *= i;
            return res;
        }
        int PowerConst( unsigned exp ) const {
            int res = 1;
            for ( unsigned p=0; p<exp; ++p ) res *= i;
            return res;
        }
};

////////////////////////////////////////////////////////////////////////////////
void test0 () {
    MFA_1arg< int, A, unsigned> mfp1( &A::Power );
    A a(2);
    std::cout << mfp1( a, 10 ) << std::endl; //2^10 = 1024
}

////////////////////////////////////////////////////////////////////////////////
void test1 () {
    A a(2);
    std::cout << mfp_1arg( &A::Power )( a,9 ) << std::endl; //2^9 = 512
    //          call helper function | operator() on the mfp_1arg object 
}

////////////////////////////////////////////////////////////////////////////////
void test2 () {
    int (A::*my_const_memfun_ptr)(unsigned int) const = &A::PowerConst;
    //                                          ===== !!!!!

    MFA_1arg_const< int, A, unsigned> mfp1( my_const_memfun_ptr );
    A a(3);
    std::cout << mfp1( a, 4 ) << std::endl; //3^4=81
}

////////////////////////////////////////////////////////////////////////////////
void test3 () {
    //MFA_1arg_const< int, A, unsigned> mfp1( &A::Power ); //should not compile
    MFA_1arg_const< int, A, unsigned> mfp1( &A::PowerConst );
    A a(5);
    std::cout << mfp1( a, 3 ) << std::endl; //5^3=125
}

////////////////////////////////////////////////////////////////////////////////
void test4 () {
    A a(10);
    std::cout << mfp_1arg( &A::PowerConst )( a,5 ) << std::endl; //100000
}

////////////////////////////////////////////////////////////////////////////////
class C {
	int i;
	public:
		C( int _i ) : i(_i)          {}
		A Multiply(const int& arg2)  { return A(i*arg2); }
		const int& Get() const       { return i; }

        C operator*( C const& arg2 ) const {
            return C(this->i * arg2.i);
        }
};
void test5 () {
	std::vector<C> v;
	for (int i=0;i<10; ++i) v.push_back( C(i) );
	std::vector<C>::const_iterator it=v.begin(), it_end=v.end();
	for ( ; it!=it_end; ++it) { std::cout << "C(" << it->Get() << ") "; }
	std::cout << std::endl;

	//second input range for transform
	std::list<int> list;
	for (int i=0;i<10; ++i) list.push_back( 10-i );
	//output range for transform (filled with garbage)
	std::vector<A> out;
	for (int i=0;i<10; ++i) out.push_back( A(0) );

	//=====================================
	std::transform( 
			v.begin(), v.end(), list.begin(), out.begin(), mfp_1arg( &C::Multiply ) );

	for ( std::vector<A>::const_iterator itA=out.begin(); itA!=out.end(); ++itA) { 
		std::cout << "A(" << itA->Get() << ") "; 
	}
	std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void test6 () {
	std::vector<C> v;
	for (int i=0;i<10; ++i) v.push_back( C(i) );
	std::vector<C>::const_iterator it=v.begin(), it_end=v.end();
	for ( ; it!=it_end; ++it) { std::cout << "C(" << it->Get() << ") "; }
	std::cout << std::endl;

	//second input range for transform
	std::list<C> list;
	for (int i=0;i<10; ++i) list.push_back( 10-i );

	//output range for transform (filled with garbage)
	std::vector<C> out;
	for (int i=0;i<10; ++i) out.push_back( C(0) );

	//=====================================
	std::transform( 
			v.begin(), v.end(), list.begin(), out.begin(), mfp_1arg( &C::operator* ) );

	for ( std::vector<C>::const_iterator itC=out.begin(); itC!=out.end(); ++itC) { 
		std::cout << "C(" << itC->Get() << ") "; 
	}
	std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
class BaseDoublePrinter {
    public:
        //virtual void PrintDouble( double const& d ) const = 0; 
        virtual void PrintDouble( double d ) const = 0; 
        virtual ~BaseDoublePrinter() {}
};
class DerivedDoublePrinter_1 : public BaseDoublePrinter {
    public:
        //virtual void PrintDouble( double const& d ) const {
        virtual void PrintDouble( double d ) const {
            std::cout << d << std::endl; //simple
        }
};
class DerivedDoublePrinter_2 : public BaseDoublePrinter {
    public:
        //virtual void PrintDouble( double const& d ) const {
        virtual void PrintDouble( double d ) const {
            std::cout << std::fixed << std::setprecision(2) << std::setw(7) << d << std::endl; //fancy
        }
};

//passing different argument to the same method
//instead of creating a second container with the method pointer duplicated -
//use bind1st
void test7 () {
    std::list< double > list_d;
    list_d.push_back( 1.0 );
    list_d.push_back( .11111111 );
    list_d.push_back( 1.222 );
    list_d.push_back( 1.0 );
    list_d.push_back( 111.22222 );

    std::cout << "Using DerivedDoublePrinter_1:\n";
    DerivedDoublePrinter_1 printer_1;
    std::for_each( list_d.begin(), list_d.end(), 
                   std::bind1st( mfp_1arg( &DerivedDoublePrinter_1::PrintDouble ), printer_1 ) );
    //                           which member function to use                      value to bound to

    std::cout << "Using DerivedDoublePrinter_2:\n";
    DerivedDoublePrinter_2 printer_2;
    std::for_each( list_d.begin(), list_d.end(), 
                   std::bind1st( mfp_1arg( &DerivedDoublePrinter_2::PrintDouble ), printer_2 ) );
    //                           which member function to use                      value to bound to
}

////////////////////////////////////////////////////////////////////////////////
// this class announces it's copy ctors allowing me to see when object is copied
//class Foo {
//        int data;
//    public:
//        Foo( int _data = 0 ) : data(_data) {}
//        //Foo( Foo const & original ) : data( original.data) { std::cout << "Foo copy\n"; }
//        Foo& operator+= ( int increment ) {
//            data += increment;
//            return *this;
//        }
//        const int& Get() const { return data; }
//
//};
//
////increment all of objects in a vector by 10 
////similar to the previous test, but now it's different objects and same 
////argument (10) passed to all of them
//
////we use the same trick as before - bind second argument of MFA_1arg to 10 with bind2nd 
////Note - since we want to increment all objects in place, adapter should be passing object by reference 
////otherwise Foo will be copied and the copy (temporary) will get incremented instead of the original
//void test8() {
//	std::vector<Foo> v;
//	for (int i=0;i<10; ++i) v.push_back( Foo(i) );
//	std::vector<Foo>::const_iterator it=v.begin(), it_end=v.end();
//	for ( ; it!=it_end; ++it) { std::cout << "Foo(" << it->Get() << ") "; }
//	std::cout << std::endl;
//
//    std::for_each( v.begin(), v.end(), 
//                   std::bind2nd( mfp_1arg( &Foo::operator+= ), 10 ) );
//
//	it=v.begin();
//	for ( ; it!=it_end; ++it) { std::cout << "Foo(" << it->Get() << ") "; }
//	std::cout << std::endl;
//}


////////////////////////////////////////////////////////////////////////////////
void test_all();

void (*pTests[])() = { test0,test1,test2,test3,test4,test5,test6,test7,test_all };

void test_all() {
	for (size_t i = 0; i<sizeof(pTests)/sizeof(pTests[0])-1; ++i) 
		pTests[i]();
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char ** argv) {
	if (argc >1) {
		int test = 0;
		std::sscanf(argv[1],"%i",&test);
		try {
            pTests[test]();
		} catch( const char* msg) {
			std::cerr << msg << std::endl;
		}
	}
	return 0;
}
