//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RC_PTR_HPP
#define RC_PTR_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <ostream>
#include <stdexcept>

/**
 * @brief Macro controlling the namespace name. Default is "memory".
 *
 */
#ifndef RC_PTR_NAMESPACE
#define RC_PTR_NAMESPACE memory
#endif

/**
 * @brief Namespace name set by RC_PTR_NAMESPACE macro. Default is "memory".
 *
 */
namespace RC_PTR_NAMESPACE
{
namespace detail
{
template<typename T, typename Deleter, typename Alloc>
class control_block
{
public:
    control_block() = delete;

    template<typename D, typename A>
    control_block(D&& deleter, A allocator) :
        m_ref_count{ 0 },
        m_weak_count{ 0 },
        m_deleter{ std::forward<D>(deleter) },
        m_allocator{ allocator }
    {
    }

    ~control_block() = default;

    std::size_t get_ref_count() const noexcept
    {
        return m_ref_count;
    }

    std::size_t get_weak_count() const noexcept
    {
        return m_weak_count;
    }

    void increase_ref_count() noexcept
    {
        ++m_ref_count;
    }

    void increase_weak_count() noexcept
    {
        ++m_weak_count;
    }

    void decrease_ref_count() noexcept
    {
        --m_ref_count;
    }

    void decrease_weak_count() noexcept
    {
        --m_weak_count;
    }

    Deleter& get_deleter() noexcept
    {
        return m_deleter;
    }

    const Deleter& get_deleter() const noexcept
    {
        return m_deleter;
    }

    Alloc get_allocator() noexcept
    {
        return m_allocator;
    }

private:
    std::size_t m_ref_count;
    std::size_t m_weak_count;
    Deleter     m_deleter;
    Alloc       m_allocator;
};
} // namespace detail

/**
 * @brief Exception thrown when rc_ptr is created from expired weak_rc_ptr.
 *
 */
struct bad_weak_rc_ptr : public std::runtime_error {
    using base = std::runtime_error;
    bad_weak_rc_ptr(std::string msg) : base{ std::move(msg) } { }
};

template<typename T, typename Deleter = std::default_delete<T>,
         typename Alloc = std::allocator<T>>
class weak_rc_ptr;

template<typename T>
class enable_rc_from_this;

/**
 * @brief rc_ptr class template manages shared ownership of an object of
 * type T via the pointer. Multiple rc_ptr objects can manage the
 * single object. The managed object is deleted when the last remaining
 * rc_ptr object is either destroyed or reset.
 *
 * rc_ptr meets CopyConstructible, CopyAssignable, MoveConstructible and
 * MoveAssignable requirements.
 *
 * rc_ptr keeps track of the reference count by allocating the control block
 * internally. By default, it is allocated and deallocated using the
 * std::allocator<T>. The control block is deallocated w1hen the count of both
 * rc_ptr and weak_rc_ptr objects managing a single object reaches zero.
 *
 * Custom deleters may be used to customize the objects
 * destruction.
 *
 * Custom allocator may be provided for internal use to allocate
 * and later deallocate the control block.
 *
 * The class methods are not thread safe.
 *
 * @tparam T Type of the managed object
 * @tparam Deleter Type of the deleter for the destruction of the managed
 * object. Default is std::default_delete<T>.
 * @tparam Alloc Type of the allocator used for allocation and deallocation of
 * the internal control block. Default is std::allocator<T>.
 */
template<typename T, typename Deleter = std::default_delete<T>,
         typename Alloc = std::allocator<T>>
class rc_ptr
{
public:
    using element_type   = std::remove_extent_t<T>;
    using pointer        = element_type*;
    using reference      = element_type&;
    using deleter_type   = Deleter;
    using allocator_type = Alloc;
    using weak_type      = weak_rc_ptr<T, deleter_type, allocator_type>;

    /**
     * @brief Default constructor. Constructs rc_ptr that owns nothing.
     *
     */
    constexpr rc_ptr() noexcept : m_ptr{ pointer() }, m_control_block{ nullptr }
    {
    }

    /**
     * @brief Constructs rc_ptr that owns nothing.
     *
     */
    constexpr rc_ptr(std::nullptr_t) noexcept :
        m_ptr{ pointer() },
        m_control_block{ nullptr }
    {
    }

    /**
     * @brief Constructs rc_ptr object from the pointer ptr. Takes ownership
     * over the pointer. Custom deleter and/or custom allocator may be
     * provided.
     *
     * @tparam D
     * @tparam A
     * @param ptr
     * @param deleter
     * @param allocator
     */
    template<typename D = deleter_type, typename A = allocator_type>
    rc_ptr(pointer ptr, D&& deleter = D{}, A allocator = A{}) :
        m_ptr{ ptr },
        m_control_block{ nullptr }
    {
        if (!m_ptr)
        {
            return;
        }

        {
            auto control_block_allocator =
                control_block_allocator_type{ allocator };
            auto mem = control_block_allocator_traits_type::allocate(
                control_block_allocator,
                1);

            assert(mem);
            control_block_allocator_traits_type::construct(
                control_block_allocator,
                mem,
                std::forward<D>(deleter),
                control_block_allocator);

            m_control_block = mem;
        }

        m_control_block->increase_ref_count();

        // Additional step for classes deriving from enable_rc_from_this.
        if constexpr (std::is_base_of_v<enable_rc_from_this<T>, T>)
        {
            ptr->m_weak = weak_rc_ptr<T>(*this);
        }
    }

    /**
     * @brief Constructs the rc_ptr from the std::unique_ptr. Stored object and
     * the deleter is transfered to the newly created rc_ptr object.
     *
     * @tparam D
     * @tparam A
     * @param ptr
     * @param allocator
     */
    template<typename D = deleter_type, typename A = allocator_type>
    rc_ptr(std::unique_ptr<T, D>&& ptr, A allocator = A{}) :
        m_ptr{ ptr.get() },
        m_control_block{ nullptr }
    {
        if (!m_ptr)
        {
            return;
        }

        {
            auto control_block_allocator =
                control_block_allocator_type{ allocator };
            auto mem = control_block_allocator_traits_type::allocate(
                control_block_allocator,
                1);

            assert(mem);
            control_block_allocator_traits_type::construct(
                control_block_allocator,
                mem,
                std::forward<D>(ptr.get_deleter()),
                control_block_allocator);

            m_control_block = mem;
        }

        m_control_block->increase_ref_count();
        ptr.release(); // Now rc_ptr owns the resource
    }

    /**
     * @brief Copy constructor.
     *
     * @param other
     */
    rc_ptr(const rc_ptr& other) noexcept :
        m_ptr{ pointer() },
        m_control_block{ nullptr }
    {
        *this = other;
    }

    /**
     * @brief Move constructor.
     *
     * @param other
     */
    rc_ptr(rc_ptr&& other) noexcept :
        m_ptr{ pointer() },
        m_control_block{ nullptr }
    {
        *this = std::move(other);
    }

    /**
     * @brief Constructs rc_ptr from weak_rc_ptr.
     *
     * @param other
     * @throws bad_weak_rc_ptr when other.expired() == true
     */
    rc_ptr(const weak_rc_ptr<T, deleter_type, allocator_type>& other) :
        m_ptr{ pointer() },
        m_control_block{ nullptr }
    {
        *this = other;
    }

    /**
     * @brief Copy assignment operator.
     *
     * @param other
     * @return rc_ptr&
     */
    rc_ptr& operator=(const rc_ptr& other) noexcept
    {
        m_control_block = other.m_control_block;
        m_ptr           = other.get();

        if (!m_control_block)
        {
            return *this;
        }

        m_control_block->increase_ref_count();
        return *this;
    }

    /**
     * @brief Move assignment operator.
     *
     * @param other
     * @return rc_ptr&
     */
    rc_ptr& operator=(rc_ptr&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        m_control_block       = other.m_control_block;
        other.m_control_block = nullptr;
        m_ptr                 = other.get();
        other.m_ptr           = pointer();
        return *this;
    }

    /**
     * @brief Assignment of weak_rc_ptr to rc_tr.
     *
     * @param other
     * @throws bad_weak_rc_ptr when other.expired() == true
     */
    rc_ptr& operator=(const weak_rc_ptr<T, deleter_type, allocator_type>& other)
    {
        if (other.expired())
        {
            throw bad_weak_rc_ptr("rc_ptr expired.");
        }

        *this = other.lock();
        return *this;
    }

    /**
     * @brief Destroys rc_ptr object. The managed object is destroyed and the
     * memory is deallocated when the last remaining rc_ptr managing the object
     * is destroyed.
     *
     */
    ~rc_ptr()
    {
        static_assert(std::is_invocable_v<deleter_type&, pointer>,
                      "Deleter must be invocable with a pointer.");

        if (!m_control_block)
        {
            assert(!m_ptr);
            return;
        }

        if (m_control_block->get_ref_count() != 1)
        {
            m_control_block->decrease_ref_count();
            return;
        }

        if (m_ptr)
        {
            get_deleter()(get());
            m_ptr = pointer();
        }

        m_control_block->decrease_ref_count();

        if (m_control_block->get_weak_count() != 0)
        {
            return;
        }

        {
            assert(m_control_block);
            auto control_block_allocator = control_block_allocator_type{
                m_control_block->get_allocator()
            };
            control_block_allocator_traits_type::destroy(
                control_block_allocator,
                m_control_block);
            control_block_allocator_traits_type::deallocate(
                control_block_allocator,
                m_control_block,
                1);
        }

        m_control_block = nullptr;
    }

    /**
     * @brief Returns the stored pointer.
     *
     * @return pointer
     */
    pointer get() const noexcept
    {
        return m_ptr;
    }

    /**
     * @brief Returns a reference to the deleter.
     *
     * @return const deleter_type&
     */
    const deleter_type& get_deleter() const noexcept
    {
        assert(m_control_block);
        return m_control_block->get_deleter();
    }

    /**
     * @brief Returns a reference to the deleter.
     *
     * @return deleter_type&
     */
    deleter_type& get_deleter() noexcept
    {
        assert(m_control_block);
        return m_control_block->get_deleter();
    }

    /**
     * @brief Returns a copy of the allocator.
     *
     * @return deleter_type&
     */
    allocator_type get_allocator() noexcept
    {
        assert(m_control_block);
        return m_control_block->get_allocator();
    }

    /**
     * @brief Returns the current number of rc_ptr objects owning the
     * resource.
     *
     * @return std::size_t
     */
    std::size_t use_count() const noexcept
    {
        return (!m_control_block) ? 0 : m_control_block->get_ref_count();
    }

    /**
     * @brief Checks whether the instance of rc_ptr is the only one managing
     * the resource.
     *
     * @return true if reference count is equal to zero
     * @return false otherwise
     */
    bool unique() const noexcept
    {
        return (use_count() == 1);
    }

    /**
     * @brief Releases the ownerhip of the managed object.
     *
     */
    void reset() noexcept
    {
        rc_ptr().swap(*this);
    }

    /**
     * @brief Swaps contents with other rc_ptr object.
     *
     * @param other
     */
    void swap(rc_ptr& other) noexcept
    {
        std::swap(m_control_block, other.m_control_block);
        std::swap(m_ptr, other.m_ptr);
    }

    /**
     * @brief Checks whether the current rc_ptr object precedes other.
     *
     * @tparam U
     * @tparam D
     * @tparam A
     * @param other
     * @return true
     * @return false
     */
    template<typename U, typename D, typename A>
    bool owner_before(const rc_ptr<U, D, A>& other) const noexcept
    {
        return other.m_control_block < m_control_block;
    }

    /**
     * @brief Checks whether the current rc_ptr object precedes other.
     *
     * @tparam U
     * @tparam D
     * @tparam A
     * @param other
     * @return true
     * @return false
     */
    template<typename U, typename D, typename A>
    bool owner_before(const weak_rc_ptr<U, D, A>& other) const noexcept
    {
        return other.m_control_block < m_control_block;
    }

    /**
     * @brief Implicit conversion to bool. Compares the stored pointer to
     * nullptr.
     *
     * @return true if stored pointer is not nullptr.
     * @return false otherwise
     */
    operator bool() const noexcept
    {
        return static_cast<bool>(get());
    }

    /**
     * @brief Dereferences the stored pointer and returns a reference to the
     * value. Undefined behaviour if the stored pointer is not valid.
     *
     * @return reference
     */
    reference operator*() const noexcept
    {
        assert(get());
        return *get();
    }

    /**
     * @brief Dereferences the stored pointer and returns a reference to the
     * value. Undefined behaviour if the stored pointer is not valid.
     *
     * @return pointer
     */
    pointer operator->() const noexcept
    {
        assert(get());
        return get();
    }

    /**
     * @brief Provides index based access to the stored array.
     *
     * @param index
     * @return element_type&
     */
    element_type& operator[](const std::ptrdiff_t index) const
    {
        assert(get());
        return get()[index];
    }

private:
    using allocator_traits = std::allocator_traits<allocator_type>;
    using control_block_type =
        detail::control_block<T, deleter_type, allocator_type>;

    using control_block_allocator_type = typename std::allocator_traits<
        allocator_type>::template rebind_alloc<control_block_type>;
    using control_block_allocator_traits_type = typename std::allocator_traits<
        allocator_type>::template rebind_traits<control_block_type>;

    friend class weak_rc_ptr<T, deleter_type, allocator_type>;

    rc_ptr(pointer ptr, control_block_type* control_block) :
        m_ptr{ ptr },
        m_control_block{ control_block }
    {
        assert(m_ptr);
        assert(m_control_block);
        m_control_block->increase_ref_count();
    }

    pointer             m_ptr;
    control_block_type* m_control_block;
};

/**
 * @brief Outputs the value of get() to the output stream.
 *
 * @tparam CharT
 * @tparam Traits
 * @tparam U
 * @tparam D
 * @tparam A
 * @param os
 * @param ptr
 * @return std::basic_ostream<CharT, Traits>&
 */
template<typename CharT, typename Traits, typename U, typename D, typename A>
std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
               const rc_ptr<U, D, A>&             ptr)
{
    return os << ptr.get();
}

/**
 * @brief weak_rc_ptr is a smart pointer that represents a weak reference to the
 * resource.
 *
 * rc_ptr object can be constructed from weak_rc_ptr object by using the
 * lock() method or by assignment.
 *
 * @tparam T
 * @tparam Deleter
 * @tparam Alloc
 */
template<typename T, typename Deleter, typename Alloc>
class weak_rc_ptr
{
public:
    using element_type   = std::remove_extent_t<T>;
    using pointer        = element_type*;
    using reference      = element_type&;
    using deleter_type   = Deleter;
    using allocator_type = Alloc;

    /**
     * @brief Default constructor. Constructs an empty weak_rc_ptr object.
     *
     */
    constexpr weak_rc_ptr() : m_ptr{ pointer() }, m_control_block{ nullptr } { }

    /**
     * @brief Copy constructor.
     *
     * @param other
     */
    weak_rc_ptr(const weak_rc_ptr& other) :
        m_ptr{ pointer() },
        m_control_block{ nullptr }
    {
        *this = other;
    }

    /**
     * @brief Move constructor.
     *
     * @param other
     */
    weak_rc_ptr(weak_rc_ptr&& other) :
        m_ptr{ pointer() },
        m_control_block{ nullptr }
    {
        *this = std::move(other);
    }

    /**
     * @brief Constructs the weak_rc_ptr from the rc_ptr.
     *
     * @param other
     */
    weak_rc_ptr(const rc_ptr<T, deleter_type, allocator_type>& other) :
        m_ptr{ pointer() },
        m_control_block{ nullptr }
    {
        if (!other.get())
        {
            return;
        }

        assert(other.m_control_block);
        m_ptr           = other.get();
        m_control_block = other.m_control_block;
        m_control_block->increase_weak_count();
    }

    /**
     * @brief Destroys weak_rc_ptr.
     *
     */
    ~weak_rc_ptr()
    {
        if (!m_control_block)
        {
            assert(!m_ptr);
            return;
        }

        m_control_block->decrease_weak_count();

        if (m_control_block->get_ref_count() != 0)
        {
            return;
        }

        if (m_control_block->get_weak_count() != 0)
        {
            return;
        }

        {
            assert(m_control_block);
            auto control_block_allocator = control_block_allocator_type{
                m_control_block->get_allocator()
            };
            control_block_allocator_traits_type::destroy(
                control_block_allocator,
                m_control_block);
            control_block_allocator_traits_type::deallocate(
                control_block_allocator,
                m_control_block,
                1);
        }

        m_control_block = nullptr;
    }

    /**
     * @brief Copy assignment operator.
     *
     * @param other
     * @return weak_rc_ptr&
     */
    weak_rc_ptr& operator=(const weak_rc_ptr& other)
    {
        if (!other.m_control_block)
        {
            assert(!other.m_ptr);
            return *this;
        }

        m_ptr           = other.m_ptr;
        m_control_block = other.m_control_block;
        m_control_block->increase_weak_count();
        return *this;
    }

    /**
     * @brief Move assignment operator.
     *
     * @param other
     * @return weak_rc_ptr&
     */
    weak_rc_ptr& operator=(weak_rc_ptr&& other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_ptr                 = other.m_ptr;
        other.m_ptr           = pointer();
        m_control_block       = other.m_control_block;
        other.m_control_block = nullptr;
        return *this;
    }

    /**
     * @brief Assigns rc_ptr to weak_rc_ptr.
     *
     * @param other
     */
    weak_rc_ptr& operator=(const rc_ptr<T, deleter_type, allocator_type>& other)
    {
        m_ptr           = other.m_ptr;
        m_control_block = other.m_control_block;

        if (!m_control_block)
        {
            return *this;
        }

        m_control_block->increase_weak_count();
        return *this;
    }

    /**
     * @brief Checks if the managed object was deleted.
     *
     * @return true
     * @return false
     */
    bool expired() const noexcept
    {
        return (!m_control_block || m_control_block->get_ref_count() == 0);
    }

    /**
     * @brief Returns a reference to the deleter.
     *
     * @return const deleter_type&
     */
    const deleter_type& get_deleter() const noexcept
    {
        assert(m_control_block);
        return m_control_block->get_deleter();
    }

    /**
     * @brief Returns a reference to the deleter.
     *
     * @return deleter_type&
     */
    deleter_type& get_deleter() noexcept
    {
        assert(m_control_block);
        return m_control_block->get_deleter();
    }

    /**
     * @brief Returns a copy of the allocator.
     *
     * @return deleter_type&
     */
    allocator_type get_allocator() noexcept
    {
        assert(m_control_block);
        return m_control_block->get_allocator();
    }

    /**
     * @brief Returns the current number of rc_ptr objects owning the
     * resource.
     *
     * @return std::size_t
     */
    std::size_t use_count() const noexcept
    {
        return (!m_control_block) ? 0 : m_control_block->get_ref_count();
    }

    /**
     * @brief Creates rc_ptr managing the stored object. If expired() == true,
     * the default constructed rc_ptr is returned.
     *
     * @return rc_ptr<T, deleter_type, allocator_type>
     */
    rc_ptr<T, deleter_type, allocator_type> lock() const noexcept
    {
        return expired() ?
                   rc_ptr<T, deleter_type, allocator_type>{} :
                   rc_ptr<T, deleter_type, allocator_type>{ m_ptr,
                                                            m_control_block };
    }

    /**
     * @brief Releases the ownerhip of the managed object.
     *
     */
    void reset() noexcept
    {
        weak_rc_ptr().swap(*this);
    }

    /**
     * @brief Swaps contents with other weak_rc_ptr object.
     *
     * @param other
     */
    void swap(weak_rc_ptr& other) noexcept
    {
        std::swap(m_control_block, other.m_control_block);
        std::swap(m_ptr, other.m_ptr);
    }

    /**
     * @brief Checks whether the current rc_ptr object precedes other.
     *
     * @tparam U
     * @tparam D
     * @tparam A
     * @param other
     * @return true
     * @return false
     */
    template<typename U, typename D, typename A>
    bool owner_before(const rc_ptr<U, D, A>& other) const noexcept
    {
        return other.m_control_block < m_control_block;
    }

    /**
     * @brief Checks whether the current rc_ptr object precedes other.
     *
     * @tparam U
     * @tparam D
     * @tparam A
     * @param other
     * @return true
     * @return false
     */
    template<typename U, typename D, typename A>
    bool owner_before(const weak_rc_ptr<U, D, A>& other) const noexcept
    {
        return other.m_control_block < m_control_block;
    }

private:
    using control_block_type =
        detail::control_block<T, deleter_type, allocator_type>;

    using control_block_allocator_type = typename std::allocator_traits<
        allocator_type>::template rebind_alloc<control_block_type>;
    using control_block_allocator_traits_type = typename std::allocator_traits<
        allocator_type>::template rebind_traits<control_block_type>;

    pointer             m_ptr;
    control_block_type* m_control_block;
};

/**
 * @brief enable_rc_from_this class template allows to safely create rc_ptr and
 * weak_rc_ptr object from this pointer.
 *
 * @tparam T
 */
template<typename T>
class enable_rc_from_this
{
protected:
    constexpr enable_rc_from_this()                 = default;
    enable_rc_from_this(const enable_rc_from_this&) = default;
    enable_rc_from_this(enable_rc_from_this&&)      = default;
    ~enable_rc_from_this()                          = default;

    enable_rc_from_this& operator=(const enable_rc_from_this&) = default;
    enable_rc_from_this& operator=(enable_rc_from_this&&) = default;

public:
    /**
     * @brief Creates the rc_ptr object from this.
     *
     * @return rc_ptr<T>
     */
    rc_ptr<T> rc_from_this()
    {
        return m_weak.lock();
    }

    /**
     * @brief Creates the rc_ptr object from this.
     *
     * @return rc_ptr<const T>
     */
    rc_ptr<const T> rc_from_this() const
    {
        return m_weak.lock();
    }

    /**
     * @brief Creates the weak_rc_ptr object from this.
     *
     * @return weak_rc_ptr<T>
     */
    weak_rc_ptr<T> weak_rc_from_this()
    {
        return m_weak;
    }

    /**
     * @brief Creates the weak_rc_ptr object from this.
     *
     * @return weak_rc_ptr<const T>
     */
    weak_rc_ptr<const T> weak_rc_from_this() const
    {
        return m_weak;
    }

private:
    friend class rc_ptr<T>;

    mutable weak_rc_ptr<T> m_weak;
};

/**
 * @brief Creates the rc_ptr instance, forwarding the arguments to the
 * constructor of type T.
 *
 * @tparam T
 * @tparam ArgsT
 * @param args
 * @return rc_ptr<T>
 */
template<typename T, typename... ArgsT>
rc_ptr<T> make_rc(ArgsT&&... args)
{
    return rc_ptr<T>{ new T{ std::forward<ArgsT>(args)... } };
}

/**
 * @brief owner_less is a function object that enables rc_ptr and weak_rc_ptr
 * owner based ordering.
 *
 * @tparam T
 * @tparam Deleter
 * @tparam Alloc
 */
template<typename T = void, typename Deleter = std::default_delete<T>,
         typename Alloc = std::allocator<T>>
struct owner_less;

/**
 * @brief
 *
 * @tparam T
 * @tparam Deleter
 * @tparam Alloc
 */
template<typename T, typename Deleter, typename Alloc>
struct owner_less<rc_ptr<T, Deleter, Alloc>> {
    bool operator()(const rc_ptr<T, Deleter, Alloc>&      lhs,
                    const weak_rc_ptr<T, Deleter, Alloc>& rhs) const noexcept
    {
        return lhs.owner_before(rhs);
    };

    bool operator()(const weak_rc_ptr<T, Deleter, Alloc>& lhs,
                    const rc_ptr<T, Deleter, Alloc>&      rhs) const noexcept
    {
        return lhs.owner_before(rhs);
    };

    bool operator()(const rc_ptr<T, Deleter, Alloc>& lhs,
                    const rc_ptr<T, Deleter, Alloc>& rhs) const noexcept
    {
        return lhs.owner_before(rhs);
    };
};

/**
 * @brief
 *
 * @tparam T
 * @tparam Deleter
 * @tparam Alloc
 */
template<typename T, typename Deleter, typename Alloc>
struct owner_less<weak_rc_ptr<T, Deleter, Alloc>> {
    bool operator()(const rc_ptr<T, Deleter, Alloc>&      lhs,
                    const weak_rc_ptr<T, Deleter, Alloc>& rhs) const noexcept
    {
        return lhs.owner_before(rhs);
    };

    bool operator()(const weak_rc_ptr<T, Deleter, Alloc>& lhs,
                    const rc_ptr<T, Deleter, Alloc>&      rhs) const noexcept
    {
        return lhs.owner_before(rhs);
    };

    bool operator()(const weak_rc_ptr<T, Deleter, Alloc>& lhs,
                    const weak_rc_ptr<T, Deleter, Alloc>& rhs) const noexcept
    {
        return lhs.owner_before(rhs);
    };
};

} // namespace RC_PTR_NAMESPACE

#endif
