
<div align="center">

  # Super Fast List
  
  Inspired by HFT order book implementations, I wanted to recreated a basic version of a container in c++ that supports super fast operations for adding and removal to test the concepts used in those order books. This list supporst an average of up **41 million ops/s**, including FIFO ops + remove access/removal. 
</div>
<br/>

## Performance 
<div align="center"> 
  <img width="900" height="484" alt="Screenshot 2026-03-06 at 2 19 18 PM" src="https://github.com/user-attachments/assets/42b57a00-c81b-4fd3-a3c6-f25f10bd9965" />
</div>
<br/>

As seen, the fast list outperforms std::list on every benchmark by a non-trivial factor.  <br>
**Randomed mixed operations (first test)** is the probably most realistic test. Slowdown is probably due to a collapse in branch prediction support, simulating real
hot path conditions.<br/>

**However, fast list supports both random access and removal** while std::list doesnt  <br>

<br/>

## List Operations and Features: 
- Add to back O(1)
- Remove from front & back O(1)
- Access and remove at random O(1)

In theory:
Could support adding to front O(1)
Could support adding to random O(1)

<br/>

## Key Optimizations:
1) **Cache locality**. Preallocates enough buffer for up to 1 million nodes contiguously leveraging cache locality in cpu access.
2) **Cache line buffering**. List node structs are designed to be 16 bytes, so each cache line fetches 4 structs 
3) **Prefault pages**. Pages are prefaulted upon allocation, ensure even faster access time as we can leverage TLB.
4) **Linked list**. Actual list is implemented using memory contiguous free lists with next and prev. Supports O(1) random, front and back removal and access.
5) **Boolean ID Array**. Uses an array of boolean fails, each array indexes uses pointer arithmetic on memory to obtain the ith list node after the buffer base. 
6) **Free list**. Dynamically append remove nodes to a free vector, allows operations to reuse memory as much as possible while keeping FIFO order.
7) **Integer** rather than pointers for next/prev. Squeezes even less bytes into a node struct.


<br/>

## More Benchmark Metrics

Hardware - Apple M2 Pro, Sonoma 14.6.1, 16GB Ram

<br/>

### Randomized — Number of operations / s
| Operation | std::list | fast_list | Speedup |
|-----------|----------:|----------:|--------:|
| Add       | 57.7M     | 451.4M    | **7.82x** |
| Remove    | 20.3M     | 238.7M    | **11.76x** |
| Consume   | 29.9M     | 279.7M    | **9.35x** |
| Mixed     | 34.3M     | 41.8M     | **1.22x** |

> `std::list` does **not** support O(1) random access removal — requires O(n) walk to find order by ID.  
> `fast_list` supports O(1) removal by `order_id` via `orderMap[]` direct lookup.

### Randomized — Time / operation (fast_list)
| Operation            | N       | Time      | ns/op |
|----------------------|--------:|----------:|------:|
| Add                  | 1000000 | 2.22 ms   | 2 ns  |
| Remove (every other) | 500000  | 2.09 ms   | 4 ns  |
| Consume              | 500000  | 1.79 ms   | 4 ns  |
| Mixed                | 1000000 | 23.92 ms  | 24 ns |
<br/>


