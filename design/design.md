# Design for Nav
## Static typing
The primitive types will be:
1. int
2. bool
3. char
4. float
5. void

Complex data types will be:
1. array
2. structs

## Arrays
```
let char[3] x = [1, 2, 3+4];
let float[2] z = [1.1, 2.2];
x[0] = 7;

```

## Variable definitions
```
let int x = 0;
let char x = 0;
let int x = -1;
let char[6] x = "hello\0";
```

## Function definitions
```
fun sum(int a, int b) int {
    return a + b;
}
```

## Switch statement
```
switch (x) {
    case 0:
        print("a");
    case 1:
        print("b");
    default:
        print("c");
}
```

## Enums
```
enum Days {
    monday,
    tuesday,
    wednesday,
    thursday,
    friday,
    saturday,
    sunday,
}
```

## Compilation
Lexing is entirely completed before moving on.
Parsing is entirely completed before moving on.
Semantic analysis is entirely completed before moving on.
Optimizaion is entirely completed before moving on.

## Ifs
```
if (cond) {

} elif (cond) {

} else {

}
```

## Fors
```
for (int x = 2; cond; ++x) {
    break;
    continue;
}
```

## Structs
```
struct Point {
    int x
    int y
    char^ id
}
```

## Pointers
` is a reference (value prime)
^ is a dereference (pull up the value)
Using these symbols that aren't used for anything else will increase
speed as it avoids ambiguity
```
let int^ x = `7;
```

## Multiple files
Files need to keep metadata because it's good for giving errors.
Functions, types, are hoisted.

## Calling functions
```
call println(6);
let int x = call add(2, 3);
```

## Arithmatic
```
x = 1 + 2

y = 3 * 7

z = 2 / 2

a = 5 - 1

z = 5 | 3

z = 1 & 3
```