#ifndef RC_PTR_HPP
#define RC_PTR_HPP

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

        template<typename T>
        struct default_delete {
            void operator()(std::remove_extent_t<T>* ptr)
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
     * @brief rc_ptr is a smart pointer that manages shared ownership of a
     * pointer on a single thread. Multiple rc_ptr objects can own the same
     * object. Reference cycles are possible and may be broken by using
     * weak_rc_ptr. Owned object is deallocated when the last remaining rc_ptr
     * object is destroyed, reset or assigned to another rc_ptr, owning other
     * object.
     *
     * Custom deleter may be supplied. By default, the object is deallocated
     * using delete expression when T is a non-array type and delete[] when T is
     * an array type.
     *
     * Using non-const member functions of rc_ptr is
     * not thread-safe due to lack of synchronization in memory access.
     *
     * Copy and move assignments is not thread-safe because reference counting
     * and memory allocation is not synchronized.
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

        template<bool Cond>
        using _Requires = detail::Requires<Cond>;

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
        template<typename Del = deleter_type, typename = _Requires<std::is_nothrow_default_constructible_v<Del> && !std::is_pointer_v<Del>>>
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
        }

        /**
         * @brief Constructs rc_ptr with a pointer ptr. Newly constructed rc_ptr
         * owns ptr. Requires that Del is nothrow CopyConstructible.
         *
         * @tparam Del
         * @param ptr
         * @param deleter
         */
        template<typename Del = deleter_type, typename = _Requires<std::is_nothrow_copy_constructible_v<Del>>>
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
        template<typename Del = deleter_type, typename = _Requires<!std::is_lvalue_reference_v<Del> && std::is_nothrow_move_constructible_v<Del>>>
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
        template<typename Del = deleter_type, typename = _Requires<std::is_lvalue_reference_v<Del>>>
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

        template<typename Del = deleter_type, typename = _Requires<std::is_lvalue_reference_v<Del>>>
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
         * @return const pointer
         */
        const pointer get() const noexcept
        {
            return m_ptr;
        }

        /**
         * @brief Returns the stored pointer.
         *
         * @return pointer
         */
        pointer get() noexcept
        {
            return m_ptr;
        }

        /**
         * @brief Returns the deleter.
         *
         * @return const deleter_type&
         */
        const deleter_type& get_deleter() const noexcept
        {
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
         * @brief Release ownerhip over resurce.
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

        pointer m_ptr;
        control_block_type* m_control_block;
    };

    template<typename T>
    class weak_rc_ptr
    {
    public:
    private:
    };

}

#endif
