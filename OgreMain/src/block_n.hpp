/*
	copyright (c) dark rune studios.  all rights reserved.
*/
#pragma once

/*
    no namespace?

    adding a namespace causes the compiler to require unnecessary
    template parameters.  i have no idea why a namespace would
    interfere with parameter deduction but leaving this code
    without a namespace for now.
*/

template <typename t>
unsigned block_bytes (unsigned size)
{
    return size * sizeof (t);
}

template <typename t1, typename t2, typename... args>
unsigned block_bytes (unsigned size)
{
    return block_bytes<t1> (size) + block_bytes <t2, args...> (size);
}

template <typename t>
void block_assign (unsigned size, void* mem, t** t1)
{
    *t1 = static_cast<t*> (mem);
    auto buffer = static_cast<unsigned char*> (mem);
}

template <typename t, typename... args_t>
void block_assign (unsigned size, void* mem, t** t1, args_t**... args)
{
    *t1 = static_cast<t*> (mem);
    auto buffer = static_cast<unsigned char*> (mem);
    auto offset = size * (sizeof (t));
    buffer += offset;
    block_assign (size, static_cast<void*>(buffer), args...);
}

template <typename t, typename... args_t>
void block_swap (unsigned idx1, unsigned idx2, t* tn)
{
    auto lhs = tn + idx1;
    auto rhs = tn + idx2;
    auto temp = std::move (*lhs);
    *lhs = std::move (*rhs);
    *rhs = std::move (temp);
}

template <typename t, typename... args_t>
void block_swap (unsigned idx1, unsigned idx2, t* t1, args_t*... args)
{
    block_swap (idx1, idx2, t1);
    block_swap (idx1, idx2, args...);
}

template <typename t>
void block_relocate (unsigned newsize, unsigned moveable, void* dst, t** t1)
{
    // move all the objects we can
    for (auto i = 0u; i < moveable; ++i)
    {
        auto* tn = static_cast<t*> (dst) + i;
        auto* to = static_cast<t*> (*t1) + i;
        new (tn) t{ std::move (*to) };
    }
    // init any remaining objects
    for (auto i = moveable; i < newsize; ++i)
    {
        auto* tn = static_cast<t*> (dst) + i;
        new (tn) t{};
    }
}

template <typename t, typename... args_t>
void block_relocate (unsigned newsize, unsigned moveable, void* dst, t** t1, args_t**... args)
{
    block_relocate (newsize, moveable, dst, t1);
    auto buffer = static_cast<unsigned char*> (dst) + block_bytes<t> (newsize);
    block_relocate (newsize, moveable, buffer, args...);
}

template <typename t>
void block_relocate_memcpyable (unsigned newsize, unsigned moveable, void* dst, t** t1)
{
    memcpy (dst, *t1, block_bytes<t> (moveable));
}

template <typename t, typename... args_t>
void block_relocate_memcpyable (unsigned newsize, unsigned moveable, void* dst, t** t1, args_t**... args)
{
    block_relocate_memcpyable (newsize, moveable, dst, t1);
    auto buffer = static_cast<unsigned char*> (dst) + block_bytes<t> (newsize);
    block_relocate_memcpyable (newsize, moveable, buffer, args...);
}

template <typename t>
void block_construct (unsigned size, t* t1)
{
    for (auto i = 0u; i < size; ++i)
        new (t1 + i) t{};
}

template <typename t, typename... args_t>
void block_construct (unsigned size, t* t1, args_t*... args)
{
    block_construct<t> (size, t1);
    block_construct (size, args...);
}

template <typename t>
void block_destruct (unsigned size, t* t1)
{
    for (auto i = 0u; i < size; ++i)
    {
        auto ptr = t1 + i;
        ptr->~t ();
    }
}

template <typename t, typename... args_t>
void block_destruct (unsigned size, t* t1, args_t*... args)
{
    block_destruct<t> (size, t1);
    block_destruct (size, args...);
}

template <typename t>
void block_free (t* t1)
{
    auto ptr = static_cast<void*>(t1);
    free (ptr);
}

template <typename t, typename... args_t>
void block_delete (unsigned& size, t* t1, args_t*... args)
{
    if (!size) return;
    block_destruct (size, t1, args...);
    block_free (t1);
    size = 0;
}

template <typename t>
void block_delete_memcpyable (unsigned& size, t* t1)
{
    if (!size) return;
    block_free (t1);
    size = 0;
}

template <typename... args_t>
void block_malloc (unsigned size, args_t**... args)
{
    auto bytes = block_bytes <args_t...> (size);
    auto mem = malloc (bytes);
    block_assign (size, mem, args...);
}

template <typename... args_t>
void block_new (unsigned size, args_t**... args)
{
    if (!size) return;
    block_malloc (size, args...);
    block_construct (size, *args...);
}

template <typename owner_t, typename t, typename... args_t>
void block_resize (owner_t& original, unsigned& size, unsigned newsize, t** t1, args_t**... args)
{
    if (newsize == 0)
    {
        block_delete (size, *t1, *args...);
        return;
    }
    auto bytes = block_bytes <t, args_t...> (newsize);
    auto mem = malloc (bytes);
    auto moveable = std::min (size, newsize);
    block_relocate (newsize, moveable, mem, t1, args...);
    block_destruct (size, *t1, *args...);
    block_free (*t1); // free the old allocation
    size = newsize;
    block_assign (size, mem, t1, args...);
}

template <typename owner_t, typename t, typename... args_t>
void block_resize_memcpyable (owner_t& original, unsigned& size, unsigned newsize, t** t1, args_t**... args)
{
    if (newsize == 0)
    {
        block_delete_memcpyable (size, *t1);
        return;
    }
    auto bytes = block_bytes <t, args_t...> (newsize);
    auto mem = malloc (bytes);
    auto moveable = std::min (size, newsize);
    block_relocate_memcpyable (newsize, moveable, mem, t1, args...);
    block_free (*t1); // free the old allocation
    size = newsize;
    block_assign (size, mem, t1, args...);
}

template <typename... args_t>
struct block_n
{
    block_n () {}
    block_n (unsigned size, args_t**... args)
    {
        block_new (size, args...);
    }
    block_n (block_n&& other) = default;
    block_n (const block_n& other) = delete;
    void operator=(const block_n&) = delete;
    void operator=(block_n&&) = delete;
};