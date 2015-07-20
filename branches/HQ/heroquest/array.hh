//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: array.hh                                                   |
//  ==================================================================

#ifndef ARRAY_HH
#define ARRAY_HH

#include <assert.h>

#define STEPSIZE 8

template <class T>
class Array {
public:
	//uninitialized array;
	Array() : pointer_(0), size_(0), capacity_(0) {
	}

	//allocate size big array
	Array(unsigned size){
		capacity_ = ceil((double)size/STEPSIZE)*STEPSIZE;
		pointer_ = new T[capacity_];
	}

	//Copy Constructor
	Array(const Array<T>& a){
		size_ = a.size_;
		capacity_ = a.capacity_;
		pointer_ = new T[capacity_];
		for (unsigned i = 0; i < size_; i++)
			pointer_[i] = a.pointer_[i];
	}

	//Destructor
	~Array(){
		if (pointer_)
			delete [] pointer_;
	}

	//get size
	unsigned size(){
		return size_;
	}

	//get capacity
	unsigned capacity(){
		return capacity_;
	}

	//array access
	T& operator[](unsigned i){
#ifdef _DEBUG
		assert(i >= 0 && i < capacity_);
#endif
		if (i >= size_)
			size_ = i+1;
		return pointer_[i];
	}

	//resize array
	void resize(unsigned size){
		//too small, allocate new memory
		if (size > capacity_){
			T* temp;
			capacity_ = (unsigned)(ceil((double)size/STEPSIZE)*STEPSIZE);
			temp = new T[capacity_];
			for (unsigned i = 0; i < size_; i++){
				temp[i] = pointer_[i];
			}
			delete [] pointer_;
			pointer_ = temp;
			return;
		}
		//too big, deallocate old memory
		//makes resize calls pretty useless
		/*
		else if (size < capacity_ - STEPSIZE){
			T* temp;
			capacity_ = (unsigned)(ceil((double)size/STEPSIZE)*STEPSIZE);
			if (size_ > size)
				size_ = size;
			temp = new T[capacity_];
			for (unsigned i = 0; i < size_; i++){
				temp[i] = pointer_[i];
			}
			delete [] pointer_;
			pointer_ = temp;
			return;
		}
		*/
		//capacity is ok
	}

	//push back one element
	void push_back(T toPush){
		resize(size_+1);
		pointer_[size_++] = toPush;
	}
	//get last element
	T& back(){
		return pointer_[size_-1];
	}

private:
	T *pointer_;
	unsigned size_;
	unsigned capacity_;
};

#endif

