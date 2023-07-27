#include <iostream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <shared_mutex>


using namespace std;

/*template <class T>
class CVQueue
{
  std::queue<T> internal;
  std::shared_mutex mutex;
  std::condition_variable_any data_cond; // объект условной переменной для синхронизации методов

public:
  void push(const T& value)
  {
    std::unique_lock<std::shared_mutex> guard(mutex);
    internal.push(value);
    data_cond.notify_one(); // пробуждаем один ожидающий поток c вызовом pop(), если таковые имеются
  }

  void pop()
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    // дожидаемся, пока в очередь добавят элемент
    data_cond.wait(lock, [this] { return !internal.empty(); }); 
    
    internal.pop();
  }

  auto size()
  {
    std::shared_lock<std::shared_mutex>(mutex);
    auto result = internal.size();
    return result;
  }
};*/





struct Node
{
  int value;
  Node* next;
  std::mutex* node_mutex;
  Node(int val)
	: value(val), next(nullptr)
  {}
};

class FineGrainedQueue
{
  Node* head;
  std::mutex* queue_mutex;
public:
  FineGrainedQueue()
  	:head(nullptr)
  {}

  ~FineGrainedQueue(){
    cout << "~" << endl;
    Node *prev, *cur;
    if(!head) return;
    queue_mutex->lock();
    prev = this->head;    
    this->head=nullptr;
    cur = prev->next;	        cout << "~" << prev->value;
    delete prev;
    if(cur) cur->node_mutex->lock();
    while (cur!=nullptr){
      prev = cur;		        cout << "~" << prev->value;
      cur = cur->next;
      if (cur){ // проверили и только потом залочили
        cur->node_mutex->lock();
      }
      prev->node_mutex->unlock();
      delete prev;
    }
    queue_mutex->unlock();    
  }

public:


  void remove(int value){
    Node *prev, *cur;
    queue_mutex->lock();

    // здесь должна быть обработка случая пустого списка
    prev = this->head;
    if(prev==nullptr){
        queue_mutex->unlock();
        return;
    }
    cur = this->head->next;

    prev->node_mutex->lock();
    // здесь должна быть обработка случая удаления первого элемента
    if(!cur){
        Node *n=prev;
        this->head=nullptr;
        prev->node_mutex->unlock();
        queue_mutex->unlock();
        delete n;
        return;
    }
    queue_mutex->unlock();

    //if (cur) // проверили и только потом залочили
    cur->node_mutex->lock();

    while (cur!=nullptr){
      if (cur->value == value){
        prev->next = cur->next;
        prev->node_mutex->unlock();
        cur->node_mutex->unlock();
        delete cur;
        return;
      }
      Node* old_prev = prev;
      prev = cur;
      cur = cur->next;
      if (cur) // проверили и только потом залочили
        cur->node_mutex->lock();
      old_prev->node_mutex->unlock();
    }
    prev->node_mutex->unlock();
  }




  void insertIntoMiddle(int val, int pos){
    Node *prev, *cur;
    queue_mutex->lock();

    // здесь должна быть обработка случая пустого списка
    prev = this->head;
    if(prev==nullptr && pos==0){
        Node *n=new Node(val);
        this->head=n;
        queue_mutex->unlock();
        return;
    }
    cur = this->head->next;

    prev->node_mutex->lock();
    // здесь должна быть обработка случая вставки первого элемента
    if(pos==0){
        Node *n=new Node(val);
        n->next=this->head;
        this->head=n;
        prev->node_mutex->unlock();
        queue_mutex->unlock();
        return;
    }
    queue_mutex->unlock();

    int N=1;

    // проверили и только потом залочили
    cur->node_mutex->lock();

    while (cur!=nullptr){
      if (N == pos){
        Node *n=new Node(val);  n->node_mutex->lock();
        Node *t=cur;    n->next=t;
        prev->next=n;
        prev->node_mutex->unlock();
        n->node_mutex->unlock();
        cur->node_mutex->unlock();
        return;
      }
      Node* old_prev = prev;
      prev = cur;
      cur = cur->next;
      if (cur) // проверили и только потом залочили
        cur->node_mutex->lock();
      old_prev->node_mutex->unlock();
      N++;
    }

    //вставка в конец
    if (N == pos){
      Node *n=new Node(val);  n->node_mutex->lock();
      prev->next=n;
      prev->node_mutex->unlock();
      n->node_mutex->unlock();
      return;
    }

    prev->node_mutex->unlock();
  }

void print(){
    cout << "{" << endl;
    Node *prev, *cur;
    queue_mutex->lock();
    prev = this->head;    
    cur = this->head->next;
    prev->node_mutex->lock();
    while (cur!=nullptr){
      cout << prev->value << endl;
      Node* old_prev = prev;
      prev = cur;
      cur = cur->next;
      if (cur) // проверили и только потом залочили
        cur->node_mutex->lock();
      old_prev->node_mutex->unlock();
    }
    prev->node_mutex->unlock();
    cout << prev->value << endl;
    cout << "}" << endl;
    queue_mutex->unlock();
}

};


int main(){
  FineGrainedQueue q;
  q.insertIntoMiddle(100, 0);
  q.insertIntoMiddle(-2, 0);
  q.insertIntoMiddle(-3, 0);
  q.insertIntoMiddle(10, 1);
  q.insertIntoMiddle(5, 0);
  q.insertIntoMiddle(11, 4);
  q.insertIntoMiddle(13, 6);
  q.print();
  return 0;
}




