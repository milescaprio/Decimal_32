#pragma once
#include <initializer_list>
#include <iostream>
typedef unsigned char utiny;
typedef signed char tiny;
#define big (size_t)0-(size_t)1
#define inrange(x, a, b) ((x) >= (a) && (x) < (b)) //EXCULSIVE OF B
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define diff(a, b) ((a) > (b) ? ((a)-(b)) : ((b)-(a)))

#define PI 3.1415926535897932384626433832795028841971693993751058209740445923078164062862089986280348253421170679811048086913282306647093844
#define B11111111 255 //some visual byte definitions
#define B11110000 240
#define B00001111 15
#define B00000000 0
#define B00000001 1
#define B00010000 16

class Decimal_32 {
private:
	utiny mantissa_[31]; //supports up to 61 digits, 2 in each but 1 leading sign (1 means negative). Not fully compressed but for efficiency
	//Was considering removing leading bit from exponent and using it as the sign, instead of taking off a digit, but decided against it
	utiny exponent_; // ranges from 0 - 255. Should be 127 when {31}.{31} (to break up fairly), 97 when {1}.{61}, 158 when {61}.{0} [default], 0 when {-96}.{158}, and 255 when {159}.{-97}
	void writeTomantissa_(int dig_i, utiny dig) { //a small function to write a digit into the mantissa member, helpful because mantissa_ contains two digits per byte
		dig_i++; //the first index is the sign of the
		mantissa_[dig_i / 2] = (dig_i % 2) ?
			((dig & B00001111) | (mantissa_[dig_i / 2] & B11110000)) :
			((dig << 4) | (mantissa_[dig_i / 2] & B00001111));
	}
	utiny readFrommantissa_(int dig_i) const {
		dig_i++; //the first index is the sign of the 
		return dig_i % 2 ?
			(mantissa_[dig_i / 2] & B00001111) :
			(mantissa_[dig_i / 2] >> 4);
	}
	void wtf(int dig_i, utiny dig) {
		writeTomantissa_(dig_i, dig);
	}
	utiny rff(int dig_i) const {
		return readFrommantissa_(dig_i);
	}
	void pos_add(Decimal_32 a, Decimal_32 b); //unsafely adds to numbers assuming they aren't signed and the exponents are already equal, then sets object to it
	void pos_subtract(Decimal_32 a, Decimal_32 b); //this also assumes that a is bigger than b
public:
	static const int DIGITS_ = 61; //Max digits that Decimal_32 can store. Just good to own this constant if need to change later //these are public for usage, but are constant
	static const int DIGITBYTES_ = 31;
	~Decimal_32();
	Decimal_32();
	Decimal_32(float d, int digits);
	Decimal_32(double d, int digits);
	Decimal_32(std::initializer_list<utiny> mantissa, utiny exponent, bool signd); //remove this constructor once we have a function double or float constructor
	Decimal_32(const Decimal_32& d);
	void display(void) const;
	bool isSigned(void) const {
		return (mantissa_[0] >> 4);
	}
	int digits(void) const {
		int space = 0;
		for (int i = 0; i < DIGITS_; i++) {//for (int i = DIGITS_ - 1; i >= 0; i--) {
			auto debuggr = rff(i);
			if (rff(i)) {
				break;
			}
			else {
				space++;
			}
		}
		return DIGITS_ - space;
	}
	void negate() {
		mantissa_[0] = (!(mantissa_[0] >> 4) << 4) | mantissa_[0] & B00001111;
	}
	void lshift(); //shift everything to left
	void lshift(utiny shift); //passed arg says how far to shift
	void rshift(); //shift everything to right
	void rshift(utiny shift); 
	utiny lspace();
	utiny rspace();
	Decimal_32 abs(void) const;
	friend Decimal_32 operator+(Decimal_32 a, Decimal_32 b); //TO DO: look into changing these to pass by reference or class member and profile/benchmark; also pos_ privates, and other operators
	friend Decimal_32 operator-(Decimal_32 a, Decimal_32 b);
	friend Decimal_32 operator*(Decimal_32 a, Decimal_32 b);
	friend Decimal_32 operator/(Decimal_32 a, Decimal_32 b);
	friend bool operator<(Decimal_32 a, Decimal_32 b);
	friend bool operator>(Decimal_32 a, Decimal_32 b);
	friend bool operator==(Decimal_32 a, Decimal_32 b);
	friend bool operator!=(Decimal_32 a, Decimal_32 b);
	friend bool operator<=(Decimal_32 a, Decimal_32 b);
	friend bool operator>=(Decimal_32 a, Decimal_32 b);
	void operator|(Decimal_32& b);
	Decimal_32 operator-() const;
};