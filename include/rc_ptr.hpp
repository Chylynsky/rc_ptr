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
#include <stdexcept>

#ifndef RC_PTR_NAMESPACE
#define RC_PTR_NAMESPACE memory
#endif

namespace RC_PTR_NAMESPACE
{
    namespace detail
    {
        template<bool Cond>
        using Requires = std::enable_if_t<Cond, int>;

        inline constexpr const char* BAD_WEAK_RC_PTR_MESSAGE{ "rc_ptr expired." };

        template<typename T>
        struct default_delete {
            void operator()(std::remove_extent_t<T>* ptr) const noexcept
            {
                if constexpr (std::is_array_v<T>)
                {
                    delete[] ptr;
                }
                else
                {
                    delete ptr;
                }
            }
        };

        template<typename T, typename Deleter>
        class control_block
        {
        public:
            using deleter_type = Deleter;
            using size_type    = std::size_t;

            control_block() : m_ref_count{ 0 }, m_weak_count{ 0 }, m_deleter{}
            {
            }

            template<typename Del>
            control_block(Del&& deleter) :
                m_ref_count{ 0 },
                m_weak_count{ 0 },
                m_deleter{ std::forward<Del>(deleter) }
            {
            }

            ~control_block() = default;

            size_type& get_ref_count() noexcept
            {
                return m_ref_count;
            }

            size_type& get_weak_count() noexcept
            {
                return m_weak_count;
            }

            deleter_type& get_deleter() noexcept
            {
                return m_deleter;
            }

        private:
            size_type m_ref_count;
            size_type m_weak_count;
            deleter_type m_deleter;
        };
    }

    /**
     * @brief Exception thrown when lock() method is called on expired
     * weak_rc_ptr.
     *
     */
    struct bad_weak_rc_ptr : public std::runtime_error {
        using base = std::runtime_error;
        bad_weak_rc_ptr(std::string msg) : base{ msg } { }
    };

    template<typename T, typename Deleter = detail::default_delete<T>>
    class weak_rc_ptr;

    template<typename T>
    class enable_rc_from_this;

    /**
     * @brief rc_ptr is a smart pointer that manages shared ownership of a
     * pointer on a single thread. Multiple rc_ptr objects can own the same
     * object. Reference cycles are possible and may be broken by using
     * weak_rc_ptr. Owned object is deallocated when the last remaining
     * rc_ptr object is destroyed, reset or assigned to another rc_ptr,
     * owning other object.
     *
     * Custom deleter may be supplied. By default, the object is deallocated
     * using delete expression when T is a non-array type and delete[] when
     * T is an array type.
     *
     * Using member functions of rc_ptr is
     * not thread-safe due to lack of synchronization in memory access.
     *
     * Copy and move assignments are also not thread-safe because reference
     * counting and memory allocation is not synchronized.
     *
     * @tparam T Type of the object being managed.
     * @tparam Deleter Type of a deleter. By default, delete is used with
     * non-array types and delete[] with array types.
     */
    template<typename T, typename Deleter = detail::default_delete<T>>
    class rc_ptr
    {
    public:
        using element_type = std::remove_extent_t<T>;
        using pointer      = element_type*;
        using reference    = element_type&;
        using deleter_type = Deleter;

        /**
         * @brief Constructs rc_ptr that owns nothing.
         *
         */
        constexpr rc_ptr() noexcept :
            m_ptr{ pointer() },
            m_control_block{ nullptr }
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
         * @brief Constructs rc_ptr with a pointer ptr. Newly constructed rc_ptr
         * owns ptr. Requires that deleter_type is nothrow DefaultConstructible
         * and is not a pointer type.
         *
         * @param ptr
         */
        template<typename Del = deleter_type, typename = detail::Requires<std::is_nothrow_default_constructible_v<Del> && !std::is_pointer_v<Del>>>
        explicit rc_ptr(pointer ptr) noexcept :
            m_ptr{ ptr },
            m_control_block{ nullptr }
        {
            if (!m_ptr)
            {
                return;
            }

            m_control_block = new control_block_type{};
            assert(m_control_block);
            m_control_block->get_ref_count()++;

            if constexpr (std::is_base_of_v<enable_rc_from_this<T>, T>)
            {
                static_assert(std::is_same_v<Del, detail::default_delete<T>>, "Custom deleters are not supported for enable_rc_from_this derived classes.");
                ptr->m_weak = weak_rc_ptr<T>(*this);
            }
        }

        /**
         * @brief Constructs rc_ptr with a pointer ptr. Newly constructed rc_ptr
         * owns ptr. Requires that Del is nothrow CopyConstructible.
         *
         * @tparam Del
         * @param ptr
         * @param deleter
         */
        template<typename Del = deleter_type, typename = detail::Requires<std::is_nothrow_copy_constructible_v<Del>>>
        rc_ptr(pointer ptr, const deleter_type& deleter) noexcept :
            m_ptr{ ptr },
            m_control_block{ nullptr }
        {
            if (!m_ptr)
            {
                return;
            }

            m_control_block = new control_block_type{ std::forward<decltype(deleter)>(deleter) };
            assert(m_control_block);
            m_control_block->get_ref_count()++;
        }

        /**
         * @brief Constructs rc_ptr with a pointer ptr. Newly constructed rc_ptr
         * owns ptr if ptr is not nullptr. Requires that Del is nothrow
         * MoveConstructible and is not a pointer type.
         *
         * This constructor participates in overload resolution only when Del is
         * not an lvalue reference.
         *
         * @param ptr
         * @param deleter
         */
        template<typename Del = deleter_type, typename = detail::Requires<!std::is_lvalue_reference_v<Del> && std::is_nothrow_move_constructible_v<Del>>>
        rc_ptr(pointer ptr, Del&& deleter) noexcept :
            m_ptr{ ptr },
            m_control_block{ nullptr }
        {
            if (!m_ptr)
            {
                return;
            }

            m_control_block = new control_block_type{ std::move(deleter) };
            assert(m_control_block);
            m_control_block->get_ref_count()++;
        }

        /**
         * @brief Constructs rc_ptr with a pointer ptr. Newly constructed rc_ptr
         * owns ptr if ptr is not nullptr.
         *
         * This constructor participates in overload resolution only when Del is
         * an lvalue reference.
         *
         * @tparam Del
         * @tparam typename
         * @param ptr
         * @param deleter
         */
        template<typename Del = deleter_type, typename = detail::Requires<std::is_lvalue_reference_v<Del>>>
        rc_ptr(pointer ptr, Del& deleter) noexcept :
            m_ptr{ ptr },
            m_control_block{ nullptr }
        {
            if (!m_ptr)
            {
                return;
            }

            m_control_block = new control_block_type{ std::forward<decltype(deleter)>(deleter) };
            assert(m_control_block);
            m_control_block->get_ref_count()++;
        }

        template<typename Del = deleter_type, typename = detail::Requires<std::is_lvalue_reference_v<Del>>>
        rc_ptr(pointer ptr, std::remove_reference_t<Del>&& deleter) = delete;

        /**
         * @brief Copy constructor. other must own a valid pointer. Increases
         * reference count by one.
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
         * @brief Move constructor. Does not increase a reference count.
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
        rc_ptr(const weak_rc_ptr<T, deleter_type>& other) :
            m_ptr{ pointer() },
            m_control_block{ nullptr }
        {
            *this = other.lock();
        }

        /**
         * @brief Copy assignment operator. other must own a valid pointer.
         * Increases reference count by one.
         *
         * @param other
         * @return rc_ptr&
         */
        rc_ptr& operator=(const rc_ptr& other) noexcept
        {
            if (!other.m_ptr)
            {
                return *this;
            }

            assert(other.m_control_block && other.use_count() != 0);
            m_control_block = other.m_control_block;
            m_ptr           = other.get();
            m_control_block->get_ref_count()++;
            return *this;
        }

        /**
         * @brief Move assignment operator. Does not increase a reference count.
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
        rc_ptr& operator=(const weak_rc_ptr<T, deleter_type>& other)
        {
            *this = other.lock();
            return *this;
        }

        /**
         * @brief Destroys rc_ptr object if reference count eaches zero.
         * Control block is deallocated only if number of weak_rc_ptr objects
         * referencing it also reaches zero.
         *
         */
        ~rc_ptr()
        {
            static_assert(std::is_invocable_v<deleter_type&, pointer>, "Deleter must be invocable with a pointer.");

            if (!m_control_block)
            {
                assert(!m_ptr);
                return;
            }

            m_control_block->get_ref_count()--;

            if (m_control_block->get_ref_count() != 0)
            {
                return;
            }

            if (m_ptr)
            {
                get_deleter()(get());
                m_ptr = pointer();
            }

            if (m_control_block->get_weak_count() != 0)
            {
                return;
            }

            delete m_control_block;
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
         * @brief Returns the deleter. Undefined behaviour when nullptr or
         * use_count() == 0.
         *
         * @return const deleter_type&
         */
        const deleter_type& get_deleter() const noexcept
        {
            assert(get());
            return m_control_block->get_deleter();
        }

        /**
         * @brief Returns the deleter.
         *
         * @return deleter_type&
         */
        deleter_type& get_deleter() noexcept
        {
            return m_control_block->get_deleter();
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
         * @brief Release ownerhip over managed object.
         *
         */
        void reset() noexcept
        {
            rc_ptr().swap(*this);
        }

        /**
         * @brief Swap contents with other rc_ptr object.
         *
         * @param other
         */
        void swap(rc_ptr& other) noexcept
        {
            std::swap(m_control_block, other.m_control_block);
            std::swap(m_ptr, other.m_ptr);
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

    private:
        using control_block_type = detail::control_block<T, deleter_type>;

        friend class weak_rc_ptr<T, Deleter>;

        rc_ptr(pointer ptr, control_block_type* control_block) :
            m_ptr{ ptr },
            m_control_block{ control_block }
        {
            assert(m_ptr);
            assert(m_control_block);
            m_control_block->get_ref_count()++;
        }

        pointer m_ptr;
        control_block_type* m_control_block;
    };

    /**
     * @brief weak_rc_ptr is a smart pointer that represents a weak (non-owning)
     * reference to the resource.
     *
     * rc_ptr object can be constructed from weak_rc_ptr object by using the
     * lock() method. Locking on expired (use_count() == 0) weak_rc_ptr results
     * in bad_weak_rc_ptr exception being thrown.
     *
     * Custom deleter may be supplied when needed to
     * provide rc_ptr type compatibility. This class never deallocates the
     * managed object, thus the supplied deleter (either custom or default) is
     * never called by weak_rc_ptr objects.
     *
     * Using member functions of weak_rc_ptr is
     * not thread-safe due to lack of synchronization in memory access.
     *
     * Copy and move assignments are also not thread-safe because reference
     * counting and memory allocation is not synchronized.
     *
     * @tparam T
     * @tparam Deleter
     */
    template<typename T, typename Deleter>
    class weak_rc_ptr
    {
    public:
        using element_type = std::remove_extent_t<T>;
        using pointer      = element_type*;
        using reference    = element_type&;
        using deleter_type = Deleter;

        /**
         * @brief Default constructor. Constructs an empty weak_rc_ptr object.
         *
         */
        constexpr weak_rc_ptr() : m_ptr{ pointer() }, m_control_block{ nullptr }
        {
        }

        /**
         * @brief Copy constructor. Constructs a copy of weak_rc_ptr. If other
         * is valid, increases weak count.
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
         * @brief Move constructor. Transfers control block ownership. Does not
         * increase weak count.
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
         * @brief Constructs a weak_rc_ptr object out of rc_ptr. Increases weak
         * count if other owns a pointer.
         *
         * @param other
         */
        weak_rc_ptr(const rc_ptr<T, Deleter>& other) :
            m_ptr{ pointer() },
            m_control_block{ nullptr }
        {
            if (!other.m_ptr)
            {
                return;
            }

            assert(other.m_control_block);
            m_ptr           = other.get();
            m_control_block = other.m_control_block;
            m_control_block->get_weak_count()++;
        }

        /**
         * @brief Destroys weak_rc_ptr. If weak count reaches zero destroys the
         * control block.
         *
         */
        ~weak_rc_ptr()
        {
            if (!m_control_block)
            {
                assert(!m_ptr);
                return;
            }

            m_control_block->get_weak_count()--;

            if (m_control_block->get_ref_count() != 0)
            {
                return;
            }

            if (m_control_block->get_weak_count() != 0)
            {
                return;
            }

            delete m_control_block;
            m_control_block = nullptr;
        }

        /**
         * @brief Copy assignment. Creates a copy of weak_rc_ptr. If other
         * is valid, increases weak count.
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
            m_control_block->get_weak_count()++;
            return *this;
        }

        /**
         * @brief Move assignment. Does not increase a weak count.
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
         * @brief Assigns rc_ptr to weak_rc_ptr. Inreases weak count when other
         * is valid.
         *
         * @param other
         */
        weak_rc_ptr& operator=(const rc_ptr<T, Deleter>& other)
        {
            if (!other.m_control_block)
            {
                assert(!other.m_ptr);
                return *this;
            }

            m_ptr           = other.m_ptr;
            m_control_block = other.m_control_block;
            m_control_block->get_weak_count()++;
            return *this;
        }

        /**
         * @brief Checks if the managed object has expired. Managed object
         * expires when its reference count reaches zero.
         *
         * @return true
         * @return false
         */
        bool expired() const noexcept
        {
            return (!m_control_block || m_control_block->get_ref_count() == 0);
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
         * @brief Creates a new rc_ptr object if expired() == false.
         *
         * @return rc_ptr<T, deleter_type>
         * @throws bad_weak_rc_ptr when calling on an expired weak_rc_ptr.
         */
        rc_ptr<T, deleter_type> lock() const
        {
            if (expired())
            {
                throw bad_weak_rc_ptr(detail::BAD_WEAK_RC_PTR_MESSAGE);
            }

            assert(m_ptr);
            assert(m_control_block);
            return rc_ptr<T, deleter_type>{ m_ptr, m_control_block };
        }

        /**
         * @brief Release ownerhip over managed object.
         *
         */
        void reset() noexcept
        {
            weak_rc_ptr().swap(*this);
        }

        /**
         * @brief Swap contents with other rc_ptr object.
         *
         * @param other
         */
        void swap(weak_rc_ptr& other) noexcept
        {
            std::swap(m_control_block, other.m_control_block);
            std::swap(m_ptr, other.m_ptr);
        }

    private:
        using control_block_type = detail::control_block<T, deleter_type>;

        pointer m_ptr;
        control_block_type* m_control_block;
    };

    /**
     * @brief Class enabling rc_ptr and weak_rc_ptr creation from this.
     *
     * @tparam T
     */
    template<typename T>
    class enable_rc_from_this
    {
        static_assert(std::is_base_of_v<enable_rc_from_this<T>, T>, "enable_rc_from_this is not a base for the supplied type.");

    protected:
        constexpr enable_rc_from_this() noexcept = default;

        enable_rc_from_this(const enable_rc_from_this&) = default;

        enable_rc_from_this(enable_rc_from_this&&) = default;

        ~enable_rc_from_this() = default;

        enable_rc_from_this& operator=(const enable_rc_from_this&) = default;

        enable_rc_from_this& operator=(enable_rc_from_this&&) = default;

    public:
        /**
         * @brief Creates a new rc_ptr object from this.
         *
         * @return rc_ptr<T>
         */
        rc_ptr<T> rc_from_this()
        {
            return m_weak.lock();
        }

        /**
         * @brief Creates a new rc_ptr object from this.
         *
         * @return rc_ptr<const T>
         */
        rc_ptr<const T> rc_from_this() const
        {
            return m_weak.lock();
        }

        /**
         * @brief Creates a new weak_rc_ptr object from this.
         *
         * @return weak_rc_ptr<T>
         */
        weak_rc_ptr<T> weak_rc_from_this()
        {
            return m_weak;
        }

        /**
         * @brief Creates a new weak_rc_ptr object from this.
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
}

#endif
