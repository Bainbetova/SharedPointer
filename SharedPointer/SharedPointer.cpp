// SharedPointer.cpp
//

#include <iostream>
#include <memory>
#include <utility>

using namespace std;

//========================== Shared Pointer

//-------------- Block

struct SharedBlockBase
{
	unsigned int referencingPointersNum = 0;
};

template<typename T>
struct SharedBlock : public SharedBlockBase
{
	template<typename ... ConstructorArgs>
	SharedBlock(ConstructorArgs&& ... inArgs)
		: SharedBlockBase(), value(forward<ConstructorArgs>(inArgs) ...) { }

	T value;
};

//-------------- Pointer

template<typename T>
class SharedPointer
{
public:
	//[1] --- Default construction
	SharedPointer()
		: SharedPointer(nullptr, nullptr) { }

	//[2] --- Copy construction
	template<typename OtherT>
	SharedPointer(const SharedPointer<OtherT>& inOther)
		: SharedPointer(static_cast<T*>(inOther.sPointer), inOther.sharedBlock) { }

	SharedPointer(const SharedPointer<T>& inOther)
		: SharedPointer(inOther.sPointer, inOther.sharedBlock) { }

	//[3] --- Copy
	void reset() {
		printState("reset start {");
		release();
		resetPointers();
		printState("reset end }");
	}

	template<typename OtherT>
	SharedPointer& operator=(const SharedPointer<OtherT>& inOther) {
		return assign(static_cast<T*>(inOther.sPointer), inOther.sharedBlock);
	}

	SharedPointer& operator=(const SharedPointer<T>& inOther) {
		return assign(inOther.sPointer, inOther.sharedBlock);
	}

	//[4] --- Access
	T* operator->() const {
		return sPointer;
	}

	T& operator*() const {
		return *sPointer;
	}

	//[5] --- Comparation
	template<typename OtherT>
	bool operator ==(const SharedPointer<OtherT>& inOther) const {
		return sPointer == inOther.sPointer;
	}

	bool operator ==(const SharedPointer<T>& inOther) const {
		return sPointer == inOther.sPointer;
	}

	operator bool() const {
		return sPointer;
	}

	//[6] --- Destroy
	~SharedPointer() {
		printState("destructor start {");
		release();
		printState("destructor end }");
	}

private:
	template<typename Type, typename ... ConstructorArgs>
	friend SharedPointer<Type> makeShared(ConstructorArgs&& ... inArgs);

	template<typename ToType, typename FromType>
	friend SharedPointer<ToType> sharedDynamicCast(const SharedPointer<FromType>& inPointer);

	SharedPointer(T* inPointer, SharedBlockBase* inNewSharedBlock)
		: sPointer(inPointer), sharedBlock(inNewSharedBlock)
	{
		printState("constructor start {");
		retain();
		printState("constructor end }");
	}

	SharedPointer<T>& assign(T* inPointer, SharedBlockBase* inSharedBlock) {
		printState("assignmen start (other type) {");
		release();
		sPointer = inPointer;
		sharedBlock = inSharedBlock;
		retain();
		printState("assignmen end (other type) }");
		return this;
	}

	void retain() {
		printState("retain start {");
		if (sharedBlock) {
			++sharedBlock->referencingPointersNum;
		}
		printState("retain end }");
	}

	void release() {
		printState("release start {");
		if (sharedBlock) {
			if (--sharedBlock->referencingPointersNum == 0) {
				delete sharedBlock;
				resetPointers();
				printState("[!!!BLOCK DESTRUCTED!!!]");
			}
		}
		printState("release end }");
	}

	void resetPointers() {
		sPointer = nullptr;
		sharedBlock = nullptr;
	}

	void printState(const char* inAdditionalMessage) {
		cout << "[SharedPointer| this: " << this << " | ";
		if (sPointer) {
			cout <<
				"pointer: " << sPointer << " | "
				"block refs num: " << sharedBlock->referencingPointersNum;
		}
		else {
			cout << "EMPTY";
		}
		cout << "]: " << inAdditionalMessage << endl;
	}

	T* sPointer = nullptr;
	SharedBlockBase* sharedBlock = nullptr;
};

//-------------- Make shared

template<typename Type, typename ... ConstructorArgs>
SharedPointer<Type> makeShared(ConstructorArgs&& ... inArgs) {
	auto* theSharedBlock = new SharedBlock<Type>(forward<ConstructorArgs>(inArgs) ...);
	return { &theSharedBlock->value, theSharedBlock };
}

//-------------- Dynamic cast shared

template<typename ToType, typename FromType>
SharedPointer<ToType> sharedDynamicCast(const SharedPointer<FromType>& inPointer)
{
	auto* theCastedPointer = dynamic_cast<ToType*>(inPointer.sPointer);
	SharedBlockBase* theSharedBlock = theCastedPointer ? inPointer.sharedBlock : nullptr;
	return { theCastedPointer, theSharedBlock };
}

//========================== Test

int main()
{
	cout << " --------------------------------------{1} " << endl;
	auto thePointerA = makeShared<int>();
	cout << " --------------------------------------{2} " << endl;
	SharedPointer<int> thePointerB = thePointerA;
	cout << " --------------------------------------{3} " << endl;

	return 0;
}