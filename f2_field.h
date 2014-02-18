//F2 field class

class F2
{
	bool value;			//0 or 1
	
	public:
		F2();			//constructor, initializes value to false
		F2(bool);		//constructor, initializes to specified value
		bool get_value();	
	
		F2 operator+(const F2&);
		F2 operator-(const F2&);
		F2 operator*(const F2&);
		F2 operator/(const F2&);
		
		friend bool operator==(const F2&, const F2&);
		friend bool operator!=(const F2&, const F2&);
		friend bool operator<(const F2&, const F2&);
		friend bool operator<=(const F2&, const F2&);
		friend bool operator>(const F2&, const F2&);
		friend bool operator>=(const F2&, const F2&);
		
		friend std::ostream& operator<<(std::ostream&, const F2&);
	
};

//constructors

F2::F2()
{
	value = 0;	
}

F2::F2(bool v)
{
	value = v;	
}

//access the value

bool F2::get_value()
{
	return value;
}


//arithmetic operators

F2 F2::operator+(const F2& other)
{
	return value ^ other.value;
}

F2 F2::operator-(const F2& other)
{
	return value ^ other.value;
}

F2 F2::operator*(const F2& other)
{
	return value & other.value;
}

F2 F2::operator/(const F2& other)
{
	return value / other.value;
}

//comparison operators

bool operator==(const F2& left, const F2& right)
{
	return (left.value == right.value);
}

bool operator!=(const F2& left, const F2& right)
{
	return !(left == right);
}

bool operator<(const F2& left, const F2& right)
{
	return (left.value < right.value);
}

bool operator<=(const F2& left, const F2& right)
{
	return (left.value <= right.value);
}

bool operator>(const F2& left, const F2& right)
{
	return (left.value > right.value);
}

bool operator>=(const F2& left, const F2& right)
{
	return (left.value >= right.value);
}

//output operator

std::ostream& operator<<(std::ostream& os, const F2& f)
{
	os << f.value;
	return os;
}
