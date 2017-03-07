#include <cstddef>
#include <algorithm>
#include <list>

// Forward declaration of publically visible classes to be defined in this file
template <class T> class UnrolledLinkedList;
template <class T, class unqualifiedT = std::remove_cv<T> > class UnrolledLinkedListIterator;

template <class T>
class UnrolledLinkedList {
	private:
		class Node {
			Node *mNext;
			T* array;
			int numElements;
			std::list< UnrolledLinkedListIterator<T>* > iterators;
			Node(size_t N) {
				mNext = nullptr;
				numElements = 0;
				array = new T[N];
				std::fill_n(array, N, defaultValue);
			}
			~Node() {
				delete[] array;
				delete mNext;
			}
		};
		Node *mHead;
		Node *mTail;
		size_t N;
		int numElements;
		T defaultValue;
		friend class UnrolledLinkedListIterator<T>;
	public:	
		typedef UnrolledLinkedListIterator<T> iterator;
		iterator begin();
		iterator end();
		iterator insert(iterator, T);
		iterator& remove(iterator&);
		UnrolledLinkedList(size_t N, T defaultValue);
		~UnrolledLinkedList();
};

// Constructor for UnrolledLinkedList
template <class T>
UnrolledLinkedList<T>::UnrolledLinkedList(size_t N, T defaultValue) 
	: N(N) , defaultValue(defaultValue) {
	mHead = new Node(N);
	return;
}

// Destructor for UnrolledLinkedList
template <class T>
UnrolledLinkedList<T>::~UnrolledLinkedList(){
	delete mHead;
	return;
}

// Returns an iterator pointing to the beginning of the list
template <class T>
UnrolledLinkedListIterator<T> UnrolledLinkedList<T>::begin(){
	return UnrolledLinkedListIterator<T>(this, mHead, 0);
}

// Returns an iterator pointing to the end of the list
template <class T>
UnrolledLinkedListIterator<T> UnrolledLinkedList<T>::end(){
	return UnrolledLinkedListIterator<T>(this, mTail, mTail->numElements);
}

// Insert an element of type T after position given by the iterator.
// Return an iterator to the inserted element
template <class T>
UnrolledLinkedListIterator<T> UnrolledLinkedList<T>::insert(UnrolledLinkedListIterator<T> iterator, T element) {
	if (iterator.currNode->numElements < N) {
		// In this case, we can just append to the end of the internal array.
		iterator.currNode->array[iterator.currNode->numElements] = element;
		iterator.currNode->numElements++;
	} else {
		// Create a new node and copy half of the array into the new node to make space
		Node *nextNode = new Node(N);
		Node *nextNextNode = iterator.currNode->nextNode;
		int moveOver = iterator.currNode->numElements/2;
		int leftOver = iterator.currNode->numElements - moveOver;
		std::copy_n(iterator.currNode->array + leftOver,
					moveOver,
					nextNode->array);

		// Copy over element into new node and properly set counters.
		nextNode->array[moveOver] = element;
		nextNode->numElements = moveOver+1;
		iterator.currNode->numElements -= moveOver;

		// Update any iterators that may have been broken
		for (auto it = iterator.currNode.iterators.begin();
				it != iterator.currNode.iterators.end(); ++it){
			if (*it->currPos >= moveOver) {
				// Need to migrate the iterator to the new node
				iterator.currNode->mNext.iterators.push_back(it);
				iterator.currNode.iterators.erase(it);
				// Need to ensure it points to the correct place
				*it->currPos -= leftOver;
			}
		}

		// Correctly set pointers to point to the new node
		nextNode->mNext = nextNextNode;
		iterator.currNode->mNext = nextNode;

		// Overwrite elements that were moved over from first node
		std::fill_n(iterator.currNode->array + iterator.currNode->numElements,
					moveOver,
					defaultValue);
	}
	return ++iterator;
}

// Remove element at location iterator from list.  Return invalidated iterator
template <class T>
UnrolledLinkedListIterator<T>& UnrolledLinkedList<T>::remove(UnrolledLinkedListIterator<T>& iterator) {
	Node *nextNode = iterator.currNode->mNext;
	if (iterator.currNode->numElements <= N/2 && nextNode) {
		// In this case we need to look ahead for elements to suction in to this node
		if (nextNode->numElements < N/2) { 
			// We can't suction more elements, we need to merge the nodes.
			// Overwrite one element to delete T
			std::copy_n(nextNode->array,
						nextNode->numElements,
						iterator.currNode->array + iterator.currNode->numElements - 1);
			Node *nextNext = nextNode->mNext;
			iterator.currNode->numElements += nextNode->numElements-1;

			// Delete and invalidate iterator of deleted element
			for (auto it = iterator.currNode.iterators.begin();
					it != iterator.currNode.iterators.end(); ++it){
				if (**it == iterator) {
					*it->currNode = nullptr;
					iterator.currNode.iterators.erase(it);
					break;
				}
			}
			// Update modified iterators
			for (auto it = nextNode.iterators.begin();
				it != nextNode.iterators.end(); ++it) {
				// Move over responsibility of iterators to this node
				iterator.currNode->iterators.push_back(*it);
				// Properly set them to point into this node
				*it->currPos += numElements;
				*it->currNode = iterator.currNode;	
			}
			// So that only this node gets deleted and it does not cascade.
			iterator.currNode->mNext->mNext = nullptr;
			delete iterator.currNode->mNext;
			iterator.currNode->mNext = nextNext;
		} else { 
			// We don't merge in next node, but just pull in one element.
			std::copy_n(nextNode->array + 1,			
						nextNode->numElements-1,
						nextNode->array);
			std::fill_n(nextNode->array + nextNode->numElements - 1,
						1,
						defaultValue);
			iterator.currNode->numElements--;
			// Update iterators
			for (auto it = iterator.currNode.iterators.begin();
				it != iterator.currNode.iterators.end(); ++it){
				if (**it.currPos > iterator.currPos) {
					**it.currPos--;
				} else if (**it.currPos == iterator.currPos) {
					**it.currNode = nullptr;
				}
			}
		}
	} else {
		// No elements will be moved across nodes since numElements > N/2
		std::copy_n(iterator.currNode->array[iterator.currPos+1],
					iterator.currNode->numElements - iterator.currPos - 1,
					iterator.currNode->array[iterator.currPos]);
		// Update any borked iterators
		for (auto it = iterator.currNode.iterators.begin();
				it!= iterator.currNodde.iterators.end(); ++it) {
			if (**it.currPos > iterator.currPos) {
				**it.currPos--;
			} else if (**it.currPoos == iterator.currPos) {
				**it.currNode = nullptr;
			}
		}
	}
	return iterator;
}

template <class T, class unqualifiedT>
class UnrolledLinkedListIterator : public std::iterator<
									std::forward_iterator_tag,	// iterator category
									unqualifiedT,				// data type
									std::ptrdiff_t				// iterator difference type
									>{
private:
	typedef typename UnrolledLinkedList<T>::Node Node;
	explicit UnrolledLinkedListIterator(UnrolledLinkedList<T>*, int);
	UnrolledLinkedListIterator(UnrolledLinkedList<T>*, Node, int);
	Node* currNode;
	int currPos;
public:
	UnrolledLinkedListIterator();
	void swap(UnrolledLinkedListIterator&) noexcept;
	UnrolledLinkedListIterator& operator++ ();
	UnrolledLinkedListIterator operator++ (int);
	template<class T2>
	bool operator!= (const UnrolledLinkedListIterator<T2>&) const;
	T& operator* () const;
	T& operator-> () const;
	operator UnrolledLinkedList<const T>() const;
};


