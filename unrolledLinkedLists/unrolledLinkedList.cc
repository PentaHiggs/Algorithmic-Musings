#include <cstddef>
#include <algorithm>

// Forward declaration of the classes to be defined in this file
template <class T> class Node;
template <class T> class UnrolledLinkedList;
template <class T> class UnrolledLinkedListIterator;

// Simple node class for use in UnrolledLinkedList.  This node class's public interface is meant
// to be identical to the interface of a normal singly linked list.
// Template Parameter:
//  T				- The class of the type that is being stored in the UnrolledLinkedList
// Private fields:
// 	mNext 	 		- A private variable containing a pointer to the next node
// 	numelements		- A private variable containing the current number of elements
//	array 			- A pointer to an array containing the contained elements, of size N
//	currentElement	- A pointer to the array element data() currently returns
// Public methods:
//  Node* next()	- A public method returning a pointer to the node containing the next
//						data item.  May be the same current node or the next one.
//	T data()		- A public method returning the node's current value
//	T data(T defaultValue)	- A public method returning the node's current value unless
//								null, in that case returning defaultValue
template <class T> 
class Node {
	private:
		friend class UnrolledLinkedList<T>;

		Node<T> *mNext;
		int numElements;
		T* currentElement;
		T defaultValue;
		T* array;
		Node(size_t N);
		~Node();

	public:
		Node<T> *next();
		T data();
		T data(T defaultValue);
};

// Private constructor for Node<T> class.
// Takes the size of the internal array (which decides the level of unrolling) as a parameter.
template <class T>
Node<T>::Node(size_t N) {
	mNext = nullptr;
	numElements = 0;
	array = new T[N];
	std::fill_n(array, N, defaultValue);
	currentElement = array;
};

// Destructor for Node<T>
template <class T>
Node<T>::~Node () {
	delete[] array;
	delete mNext;
};

// Returns a pointer to the "next" node.  In the case that the next element of the list
// is actually in the same array in this node, then returns this node with a counter
// variable augmented.
template <class T>
Node<T> *Node<T>::next() {
	currentElement++;
	if ((currentElement - array) == numElements) {
		currentElement = array;
		// Just to ensure a valid state in case we messed up somewhere
		mNext->currentElement = mNext->array;
		return mNext;
	} else {
		return this;
	}
};


// Returns the data contained by the "current" node, a location within the actual node array.
template <class T>
T Node<T>::data() {
	return *currentElement;
};

// Returns the data contained by the "current" node unless it is empty,
// in which case it returns the passed in defaultValue.
template <class T>
T Node<T>::data(T defaultValue) {
	if ((array - currentElement) < numElements) {
		return *currentElement;
	} else {
		return defaultValue;
	}
};

// Generic class implementing an Unrolled Linked List.  Meant to have the exact same
// interface as a normal singly linked list.
// Template Parameter:
//	T
// Private Fields:
//  mHead		- Private variable containing a pointer to the first Node<T> of the list
//  N			- Degree of unrolling to be employed in the list, size of internal array
//					used in the Node<T>s that make up the list.
// Public Methods:
//	insert		- Inserts the given element at location given by a pointer to the node 
//					curr_node and returns the following node.
//	remove		- Removes node at location given by a pointer to the node curr_node
//					and returns the node that followed it.
//	head		- Returns the head of the list
//	Constructor	- Initializes an unrolled linked list, with internal array size N.
//
template<class T>
class UnrolledLinkedList {
	private:
		Node<T> *mHead;
		size_t N;
		friend class UnrolledLinkedListIterator<T>;
	public:

		Node<T> *insert(T element, Node<T> *curr_node);
		Node<T> *remove(Node<T> *curr_node);
		Node<T> *head();
		UnrolledLinkedList(size_t N);
		~UnrolledLinkedList();
};

// Destructor for UnrolledLinkedList
template <class T>
UnrolledLinkedList<T>::~UnrolledLinkedList(){
	delete mHead;	 
};

// Constructor for UnrolledLinkedList, takes in degree of unrolling (the
// size of the internal array) as a parameter.
template <class T>
UnrolledLinkedList<T>::UnrolledLinkedList(size_t N) : N(N) {
	head = new Node<T>(N);
};

// Returns the head node of the list
template <class T>
Node<T> *UnrolledLinkedList<T>::head() {
	// Reset the internal counter to the beginning of internal array
	mHead->currentElement = mHead->array;
	return mHead;
};

// Insert element after location and return node containing element just inserted
template <class T>
Node<T> *UnrolledLinkedList<T>::insert(T element, Node<T> *curr_node) {
	if (curr_node->numElements < N) {
		// In this case, we can just append to the end of the array.
		curr_node->array[curr_node->numElements] = element;
		curr_node->numElements++;
		curr_node->currentLocation++;
		return curr_node;
	} else {
		// Create new node and copy half of the array into the new node
		Node<T> *new_node = new Node<T>(N);
		size_t moveOverElements = curr_node->numElements/2; // x/2 rounded down
		size_t leftOverElements = curr_node->numElements - moveOverElements;
		std::copy_n(curr_node->array + leftOverElements,
					moveOverElements,
					new_node->array);
		// Copy over element into new node and properly set counters.
		new_node->array[moveOverElements] = element;
		new_node->numElements += moveOverElements + 1;
		new_node->currentLocation += moveOverElements;

		// Correclty set pointers to new node
		Node<T> *old_next = curr_node->mNext;
		new_node->mNext = old_next;
		curr_node->mNext = new_node;

		// Delete elements that were moved over from first node
		curr_node->currentLocation = curr_node->currentLocation - moveOverElements;
		curr_node->numElements = leftOverElements;
		std::fill_n(curr_node->array + curr_node->numElements,
					moveOverElements,
					curr_node->defaultValue);
		return new_node;
	}
};

// Remove element at location from list.  Return pointer to next element
template <class T>
Node<T>* UnrolledLinkedList<T>::remove(Node<T> *curr_node) {
	if (curr_node->numElements <= N/2 && curr_node->mNext) {
		// In this case we need to look ahead for elements
		if (curr_node->mNext->numElements < N/2) {
			// We can't take any more elements, hence we need to merge
			// We are purposely overwriting the current element
			std::copy_n(curr_node->mNext->array,
						curr_node->mNext->numElements,
						curr_node->currentElement);
			Node<T> *next_next = curr_node->mNext->mNext;
			curr_node->numElements += curr_node->mNext->numElements - 1;

			// So that only this node gets deleted and it does not cascade.
			curr_node->mNext->mNext = nullptr;
			delete curr_node->mNext;
			curr_node->mNext = next_next;
			return curr_node->next();
			} else {
			// We are siphoning off an element from beginning of the next node.
			*curr_node->currentElement = *curr_node->mNext->array;
			// Shift second array back
			std::copy_n(curr_node->mNext->array + 1,
						curr_node->numElements - 1, 
						curr_node->mNext->array);
			curr_node->mNext->numElements--;
			std::fill_n(curr_node->mNext->array + curr_node->mNext->numElements,
						1,
						curr_node->defaultValue);
			return curr_node;
			}
	} else {
		std::fill_n(curr_node->currentElement, 1, curr_node->defaultValue);
		curr_node->numElements--;
		curr_node->currentLocation--;	
		if (curr_node->currentLocation == curr_node->array) return nullptr;
		else return curr_node;
	}
};


int main(int args, char** argv){
	auto list = new UnrolledLinkedList<int>(73);

	for (int i=0; i < 10844; i++) {
		//list.insert(i); // It seems like it would make sense to make insert return the next elemetn
		//list.next();
	return 0;
	}
};
