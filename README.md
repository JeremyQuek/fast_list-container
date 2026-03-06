
<div align="center">

# Super Fast List

Inspired by HFT order book implementations, I wanted to recreated a basic version of a container in c++ that supports super fast operations for adding and removal to test the concepts of use in those order books. This list supporst an average of up 25 million operations / second.
  
<img width="900" height="598" alt="Screenshot 2026-03-06 at 1 49 01 AM" src="https://github.com/user-attachments/assets/b0fb7cc0-1f85-4be4-a28e-9e74af1be0f3" />

  
</div>





## List Operations and Features: 
- Add to back O(1)
- Remove from front & back O(1)
- Access and remove at random O(1)

In theory:
Could support adding to front O(1)
Could support adding to random O(1)

<br/>

## Benchmarks


### Number of operations / s

| Operation | std::list | fast_list | Speedup |
|-----------|----------:|----------:|--------:|
| Add       | 12.7M     | 72.0M     | **5.7x** |
| Remove    | 8.2M      | 19.8M     | **2.4x** |
| Consume   | 12.2M     | 22.3M     | **1.8x** |
| Mixed     | 7.1M      | 18.7M     | **2.6x** |

> `std::list` does **not** support O(1) random access removal — requires O(n) walk to find order by ID.  
> `fast_list` supports O(1) removal by `order_id` via `orderMap[]` direct lookup.

### Time / operations 

| Operation       | N       | Time     | ns/op |
|-----------------|--------:|---------:|------:|
| Add             | 1000000 | 13.88 ms | 14 ns |
| Remove (every other) | 500000 | 25.21 ms | 50 ns |
| Consume         | 500000  | 22.43 ms | 45 ns |
| Mixed           | 1000000 | 53.53 ms | 54 ns |


<br/>

## Key Optimization Features:
1) **Cache locality**. Preallocates enough buffer for up to 1 million nodes contiguously leveraging cache locality in cpu access.
2) **Cache line buffering**. List node structs are designed to be 16 bytes, so each cache line fetches 4 structs 
3) **Prefault pages**. Pages are prefaulted upon allocation, ensure even faster add time as we can avoid page faults and TLB misses
4) **Linked list**. Actual list is implemented using memory contiguous free lists with next and prev. Supports O(1) random, front and back removal and access.
5) **Boolean ID Array**. Uses an array of boolean fails, each array indexes uses pointer and memroy arithmetic to obtain the ith list node after the buffer start memory location
6) **Free list**. Dynamically append remove nodes to a free vector, allows operations to reused memory as much as possible upon removing while keeping FIFO order.
7) **Integer** rather than pointers. Squeezes even less bytes into a node struct. 



