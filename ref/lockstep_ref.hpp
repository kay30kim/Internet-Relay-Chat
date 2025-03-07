#ifndef LOCKSTEP_LOCKSTEP_HPP
#define LOCKSTEP_LOCKSTEP_HPP

#include <experimental/tuple>
#include <type_traits>

namespace lockstep {

namespace _detail {

auto get_begin_iterators ()
{
    return std::forward_as_tuple();
}

template <typename ContainerHead, typename ... ContainerTypes>
auto get_begin_iterators (ContainerHead && head, ContainerTypes && ... containers)
{
    return std::tuple_cat(std::forward_as_tuple(head.begin()), get_begin_iterators(containers...));
}

void pre_increment_iterators ()
{
    // Purposefully empty
}

template <typename IteratorHead, typename ... IteratorTypes>
void pre_increment_iterators (IteratorHead && head, IteratorTypes && ... iterators)
{
    ++head;
    pre_increment_iterators(iterators...);
}

} // namespace _detail

template <typename ... ContainerTypes>
class lockstep_container
{
    using value_type = std::tuple<typename std::add_lvalue_reference<typename std::remove_reference<ContainerTypes>::type::value_type>::type...>;
    using storage_type = std::tuple<ContainerTypes...>; // Use std::add_lvalue_reference?

 public:
    lockstep_container (ContainerTypes&& ... containers);

    template <typename Fn>
    void inspect_containers (Fn && fn);

    value_type first() const;


    class iterator
    {
        using iterator_tuple = std::tuple<typename std::remove_reference<ContainerTypes>::type::iterator...>;
        using reference = value_type;
        using pointer = typename std::add_pointer<value_type>::type;
     public:
        iterator (storage_type& storage)
          : _storage(storage)
          , _iterators(std::experimental::apply([](auto&& ... containers) { return _detail::get_begin_iterators(containers...); }, storage))
        { }

        iterator (const iterator& other)
          : _storage(other._storage)
          , _iterators(other._iterators)
        { }

        ~iterator () { }

        iterator& operator= (const iterator& other)
        {
            if (this != &other)
            {
                this->_storage = other._storage;
                this->_iterators = other._iterators;
            }
            return *this;
        }

        iterator& operator++ ();
        reference operator* () const;
        pointer operator-> () const;

     private:
        storage_type& _storage;
        iterator_tuple _iterators;
    };

    iterator begin();
    iterator end();

 private:
    storage_type _containers;
};

template <typename ... ContainerTypes>
lockstep_container<ContainerTypes...> lockstep (ContainerTypes&& ... containers)
{
    return lockstep_container<ContainerTypes...>{containers...};
}

template <typename ... ContainerTypes>
lockstep_container<ContainerTypes...>::lockstep_container (ContainerTypes&& ... containers)
  : _containers(std::forward_as_tuple(containers...))
{ }

template <typename Fn>
void inspect_containers_impl(Fn && fn)
{ }

template <typename Fn, typename ContainerHead, typename ... KontainerTypes>
void inspect_containers_impl(Fn && fn, ContainerHead && head, KontainerTypes&& ... containers)
{
    fn(head);
    inspect_containers_impl(fn, containers...);
}

template <typename ... ContainerTypes>
template <typename Fn>
void lockstep_container<ContainerTypes...>::inspect_containers (Fn && fn)
{
    auto l = [&fn](auto&& ... containers)
    {
        inspect_containers_impl(fn, containers...);
    };
    std::experimental::apply(l, _containers);
}

auto lockstep_tuple_builder ()
{
    return std::forward_as_tuple();
}

template <typename IteratorHead, typename ... IteratorTypes>
auto lockstep_tuple_builder (IteratorHead && head, IteratorTypes && ... iterators)
{
    return std::tuple_cat(std::forward_as_tuple(*(head)), lockstep_tuple_builder(iterators...));
}

template <typename ... ContainerTypes>
typename lockstep_container<ContainerTypes...>::value_type lockstep_container<ContainerTypes...>::first () const
{
    auto l = [](auto&& ... iterators)
    {
        return lockstep_tuple_builder(iterators...);
    };

    auto g = [](auto&& ... containers)
    {
        return _detail::get_begin_iterators(containers...);
    };
    return std::experimental::apply(l, std::experimental::apply(g, _containers));
}

template <typename ... ContainerTypes>
typename lockstep_container<ContainerTypes...>::iterator lockstep_container<ContainerTypes...>::begin ()
{
    lockstep_container<ContainerTypes...>::iterator it(_containers);
    return it;
}

template <typename ... ContainerTypes>
typename lockstep_container<ContainerTypes...>::value_type lockstep_container<ContainerTypes...>::iterator::operator* () const
{
    auto l = [](auto&& ... iterators)
    {
        return lockstep_tuple_builder(iterators...);
    };
    return std::experimental::apply(l, _iterators);
}

template <typename ... ContainerTypes>
typename lockstep_container<ContainerTypes...>::iterator& lockstep_container<ContainerTypes...>::iterator::operator++ ()
{
    auto l = [](auto&& ... iterators)
    {
        return _detail::pre_increment_iterators(iterators...);
    };
    std::experimental::apply(l, _iterators);
    return *this;
}

} // namespace lockstep

#endif // LOCKSTEP_LOCKSTEP_HPP