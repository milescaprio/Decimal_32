#include "Decimal_32.h"

//private
void Decimal_32::pos_add(Decimal_32 a, Decimal_32 b){
	utiny carry = 0;

	for (int i = DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit + b_currentDigit + carry;
		if (c_currentDigit > 9/*Max number in decimal*/) {
			carry = 1;
			c_currentDigit -= 10;
		}
		else {
			carry = 0; //clear carried number
		}
		wtf(i, c_currentDigit);
	}
	if (carry == 1) { //this means the number is too high for the current mantissa, need to scootch everything over
		for (int i = DIGITS_ - 1; i >= 0 + 1/*because it is scootching it shouldn't go past 1*/; i--) {
			wtf(i, rff(i - 1));
		}
		wtf(0, 1);
		if (mantissa_ < 255) mantissa_++; //TO DO how should it handle if the mantissa is already maxed
	}
}

void Decimal_32::pos_subtract(Decimal_32 a, Decimal_32 b) {
	utiny borrow = 0;
	if (b > a) { //wont infinite loop because these get swapped, but -- TO DO: AVOID COMPARISON TWICE (passed flag, seperate function, template, in this function?
		pos_subtract(b, a);
		negate();
		return;
	}
	for (int i = DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit - b_currentDigit - borrow;
		if (c_currentDigit < 0/*underflow for subtracting, have to borrow*/) {
			borrow = 1;
			c_currentDigit += 10;
		}
		else {
			borrow = 0; //clear carried number
		}
		wtf(i, c_currentDigit);
	}
}


//public
Decimal_32::~Decimal_32() {
	//does nothing
}

Decimal_32::Decimal_32() {
	for (int i = 0; i < 31; i++) {
		fraction_[i] = B00000000;
	}
	mantissa_ = 158;
}

Decimal_32::Decimal_32(float d, int digits) {
	std::cout << "I will implement this function later\n";
}

Decimal_32::Decimal_32(double d, int digits) {
	std::cout << "I will implement this function later\n";
}

Decimal_32::Decimal_32(std::initializer_list<utiny> fraction, utiny mantissa, bool signd) {
	mantissa_ = mantissa;
	for (int i = 0; i < fraction.size(); i++) {
		wtf(i + DIGITS_ - fraction.size(), fraction.begin()[i]);
	}
	if ((DIGITS_ - fraction.size() - 1) % 2) {
		wtf(DIGITS_ - fraction.size() - 1, 0);
	}
	for (int i = (DIGITS_ - fraction.size() + 1/*Offset for sign*/ - 1/*it's an index*/ - 1/*We go one back*/) / 2; i >= 0; i--) {
		fraction_[i] = B00000000;
	}
	fraction_[0] = signd ? B00010000 : B00000000;
}

Decimal_32::Decimal_32(const Decimal_32& d) {
	for (int i = 0; i < DIGITBYTES_; i++) {
		fraction_[i] = d.fraction_[i]; //obv just copies bytes over
	}
	mantissa_ = d.mantissa_;
}

void Decimal_32::display(void) const {
	auto digitscache = digits();
	int dplocation = (int)mantissa_ - 158 + digitscache; 
	int dpiterlocation = DIGITS_ + dplocation - digitscache; 
	if (isSigned()) {
		std::cout << '-';
	}
	if (dplocation <= 0) {
		std::cout << '.';
		for (int _ = dplocation; _ <= 0; _++) { //use an underscore if the variable is not needed within the loop but just to keep track of it
			std::cout << '0';
		}
		for (int i = DIGITS_ - digitscache; i < DIGITS_; i++) {
			std::cout << (int)rff(i);
		}
	}
	else {
		for (int i = DIGITS_ - digitscache; i < DIGITS_; i++) { //more efficient alternative if there is an integrated decimal point
			if (dpiterlocation == i) {
				std::cout << '.';
			}
			std::cout << (int)rff(i);
		}
	}
}

Decimal_32 Decimal_32::abs(void) const {
	Decimal_32 ret = *this;
	ret.fraction_[0] = B00001111 & ret.fraction_[0]; //clear leading sign half-byte
	return ret;
}

Decimal_32& Decimal_32::operator|(Decimal_32& b) {
	//Count avaiable shifting slots for both numbers
	int spacea = 0;
	int spaceb = 0;
	for (int i = 1; i < /*a.*/DIGITS_; i++) {
		if (!(i % 2 ? /*a.*/fraction_[i / 2] & B00001111 : (/*a.*/fraction_[i / 2]) >> 4)) {
			spacea++;
		}
		else {
			break;
		}
	}
	for (int i = 1; i < b.DIGITS_/*Using B here because b is used in the rest of the function*/; i++) {
		if (!(i % 2 ? b.fraction_[i / 2] & B00001111 : (b.fraction_[i / 2]) >> 4)) {
			spaceb++;
		}
		else {
			break;
		}
	}
	bool adderbool = ((int)/*a.*/mantissa_ - spacea < (int)b.mantissa_ - spaceb); //inverted because lesser should be the adder; then it will offset both and add
	Decimal_32& adder = adderbool ? *this : b;
	Decimal_32& addee = adderbool ? b : *this;
	int mantissaDifference = diff(/*a.*/mantissa_, b.mantissa_);
	//offset the adder
	int addeeOffset = min(mantissaDifference, (!adderbool) ? spacea : spaceb);
	for (int i = 0; i < addee.DIGITS_ - addeeOffset; i++) { //goes through and shifts
		addee.wtf(i, addee.rff(i + addeeOffset));
	}
	for (int i = addee.DIGITS_ - addeeOffset; i < addee.DIGITS_; i++) { // flushes after data
		addee.wtf(i, 0); 
	}
	addee.mantissa_ -= addeeOffset; //TO DO consider max/min mantissi
	int adderOffset = mantissaDifference - addeeOffset;
	if (adderOffset) {
		for (int i = adder.DIGITS_ - 1; i >= adderOffset; i--) { //goes through and shifts
			adder.wtf(i, adder.rff(i - adderOffset));
		}
		for (int i = adderOffset - 1; i >= 0; i--) { // flushes after data
			adder.wtf(i, 0); 
		}
	}
	adder.mantissa_ += adderOffset; //TO DO consider max/min mantissi
	return adder;
}

Decimal_32 operator+(Decimal_32 a, Decimal_32 b) {
	Decimal_32 c;
	a | b; // TO DO: use the output of the space search to quit adding to save time
	c.mantissa_ = a.mantissa_;

	bool signa = a.fraction_[0] >> 4;
	bool signb = b.fraction_[0] >> 4;
	if (!signa && signb) {
		c.pos_subtract(a, b);
		return c;
	}
	if (signa && !signb) {
		c.pos_subtract(b, a);
		return c;
	}
	if (signa && signb) {
		c.pos_add(a, b);
		c.negate();
		return c;
	}

	utiny carry = 0;
	for (int i = c.DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit + b_currentDigit + carry;
		if (c_currentDigit > 9/*Max number in decimal*/) {
			carry = 1;
			c_currentDigit -= 10;
		}
		else {
			carry = 0; //clear carried number
		}
		c.wtf(i, c_currentDigit); 
	}
	if (carry == 1) { //this means the number is too high for the current mantissa, need to scootch everything over
		for (int i = c.DIGITS_ - 1; i >= 0 + 1/*because it is scootching it shouldn't go past 1*/; i--) {
			c.wtf(i, c.rff(i - 1));
		}
		c.wtf(0, 1);
		if (c.mantissa_ < 255) c.mantissa_++; //TO DO how should it handle if the mantissa is already maxed
	}
	return c;
}

Decimal_32 operator-(Decimal_32 a, Decimal_32 b) {
	Decimal_32 c;
	a | b; // TO DO: use the output of the space search to quit adding to save time
	c.mantissa_ = a.mantissa_;
	
	bool signa = a.fraction_[0] >> 4;
	bool signb = b.fraction_[0] >> 4;
	if (!signa && signb) {
		c.pos_add(a, b);
		return c;
	}
	if (signa && !signb) { 
		c.pos_add(a, b);
		c.negate();
		return c;
	}
	if (signa && signb) { 
		c.pos_subtract(a, b);
		c.negate();
		return c; 
	}

	utiny borrow = 0;

	for (int i = c.DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit - b_currentDigit - borrow;
		if (c_currentDigit < 0/*underflow for subtracting, have to borrow*/) {
			borrow = 1;
			c_currentDigit += 10 ;
		}
		else {
			borrow = 0; //clear carried number
		}
		c.wtf(i, c_currentDigit);
	}
	//cannot be left with leftover borrow, or it would mean the b was bigger than be, contradicting the original statement
	return c;
}

bool operator<(Decimal_32 a, Decimal_32 b) { //>, >=, <= are missing comments but they are just variations of these comments
	//two odd lines of logic here, which are IFF'ed.
	//we have smaller and bigger mantissas, and if they are in order (smaller, bigger) to (a,b), then this yields true, and other way is false.
	//if the smaller mantissa is the smaller number, then this yields true, and the other way is false.
	Decimal_32& biggerexp = a.mantissa_ < b.mantissa_ ? b : a;
	Decimal_32& smallerexp = a.mantissa_ < b.mantissa_ ? a : b; //inverted logic not sign bc of edge cases like =
	bool correct = a.mantissa_ < b.mantissa_;

	utiny mantissadiff = biggerexp.mantissa_ - smallerexp.mantissa_;
	//the iterators do NOT LINE UP in the following loops; they couldn't without weirdness. The first is j for biggerexp, then the next two are i's for smallerexp.
	for (int j = 0; j < mantissadiff; j++) {
		if (biggerexp.rff(j) != 0) {
			return correct; //biggerexp is bigger, has leading numbers that smallerexp doesn't have
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - mantissadiff; i++) { //check along the aligned digits
		if (smallerexp.rff(i) < biggerexp.rff(i + mantissadiff)) {
			return correct; //an aligned digit is smaller
		}
	}
	for (int i = smallerexp.DIGITS_ - mantissadiff; i < smallerexp.DIGITS_; i++) {
		if (biggerexp.rff(i) != 0) {
			return !correct; //smallerexp is bigger, has trailing numbers that biggerexp doesn't have
		}
	}
	return !correct;
}

bool operator>(Decimal_32 a, Decimal_32 b) { //see comments on operator<.
	Decimal_32& biggerexp = a.mantissa_ > b.mantissa_ ? a : b; //now doing everything in >. Note that edge cases will now swap whether being biggerexp or smaller, but that's intended.
	Decimal_32& smallerexp = a.mantissa_ > b.mantissa_ ? b : a;
	bool correct = a.mantissa_ > b.mantissa_; //SWAPPED THIS, THE NEW CORRECT ORDER. EVERYTHING ELSE CHECKS THE SAME; SMALLER SHOULD BE SMALLER, AND VICE VERSA, EXCEPT THE EDGE CASES ARE FLIPPED (see above).

	utiny mantissadiff = biggerexp.mantissa_ - smallerexp.mantissa_;
	for (int j = 0; j < mantissadiff; j++) {
		if (biggerexp.rff(j) != 0) {
			return correct;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - mantissadiff; i++) { //check along the aligned digits
		if (biggerexp.rff(i + mantissadiff) > smallerexp.rff(i)) { //this is still the same but I just swapped around the expression because we are using the > operator in this function and this emphasizes it.
			return correct;
		}
	}
	for (int i = smallerexp.DIGITS_ - mantissadiff; i < smallerexp.DIGITS_; i++) {
		if (biggerexp.rff(i) != 0) {
			return !correct;
		}
	}
	return !correct;
}

Decimal_32 Decimal_32::/*Is not a friend function, stays in scope*/operator-() const {
	Decimal_32 ret = *this;
	ret.fraction_[0] = ((ret.fraction_[0] ^ B11111111) & B11110000) | ret.fraction_[0]; //clear leading sign half-byte
	return ret;
}