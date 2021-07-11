#pragma once
#include "script_value.hpp"

namespace scripting
{
	struct array_key
	{
		bool is_string = false;
		bool is_integer = false;
		unsigned int index{};
		std::string key{};
	};

	class array_value : public script_value
	{
	public:
		array_value(unsigned int, unsigned int);
		void operator=(const script_value&);
	private:
		unsigned int id_;
		unsigned int parent_id_;
	};

	class array final
	{
	public:
		array();
		array(const unsigned int);

		array(std::vector<script_value>);
		array(std::unordered_map<std::string, script_value>);

		array(const array& other);
		array(array&& other) noexcept;

		~array();

		array& operator=(const array& other);
		array& operator=(array&& other) noexcept;

		void add() const;
		void release() const;

		std::vector<array_key> get_keys() const;
		unsigned int size() const;

		unsigned int push(script_value) const;
		void erase(const unsigned int) const;
		void erase(const std::string&) const;
		script_value pop() const;

		script_value get(const array_key&) const;
		script_value get(const std::string&) const;
		script_value get(const unsigned int) const;

		void set(const array_key&, const script_value&) const;
		void set(const std::string&, const script_value&) const;
		void set(const unsigned int, const script_value&) const;

		unsigned int get_entity_id() const;

		unsigned int get_value_id(const std::string&) const;
		unsigned int get_value_id(const unsigned int) const;

		entity get_raw() const;

		array_value operator[](const int index) const
		{
			return {this->id_, this->get_value_id(index)};
		}

		array_value operator[](const std::string& key) const
		{
			return {this->id_, this->get_value_id(key)};
		}
	private:
		unsigned int id_;
	};
}
