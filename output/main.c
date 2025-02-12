typedef struct Node {
	 int  value ;
	Node* prev ;
	
} Node;

typedef struct Stack {
	Node* tail ;
	 int  length ;
	
} Stack;

void push( Stack* s, int value) {
  Node* n = &(Node){value, nil};
  ++s->length;
  if (s->length==1) {
    s->tail = n;
    return ;
  }

  n->prev = s->tail;
  s->tail = n;
}

int pop( Stack* s) {
  if (s->length==0) {
    return 0;
  }

  --s->length;
  if (s->length==0) {
    Node* tail = s->tail;
    s->tail = nil;
    return tail->value;
  }

  Node* tail = s->tail;
  s->tail = s->tail->prev;
  return tail->value;
}

int peek( Stack* s) {
  if (s->tail==nil) {
    return 0;
  }

  return s->tail->value;
}

