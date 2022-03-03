#include "Decimal_32.h"

//private
void Decimal_32::pos_add(Decimal_32 a, Decimal_32 b){
	utiny carry = 0;

	for (int i = DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rfm(i);
		utiny b_currentDigit = b.rfm(i);
		utiny c_currentDigit = a_currentDigit + b_currentDigit + carry;
		if (c_currentDigit > 9/*Max number in decimal*/) {
			carry = 1;
			c_currentDigit -= 10;
		}
		else {
			carry = 0; //clear carried number
		}
		wtm(i, c_currentDigit);
	}
	if (carry == 1) { //this means the number is too high for the current exponent, need to scootch everything over
		for (int i = DIGITS_ - 1; i >= 0 + 1/*because it is scootching it shouldn't go past 1*/; i--) {
			wtm(i, rfm(i - 1));
		}
		wtm(0, 1);
		if (exponent_ < 255) exponent_++; //TO DO how should it handle if the exponent is already maxed
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
		utiny a_currentDigit = a.rfm(i);
		utiny b_currentDigit = b.rfm(i);
		int c_currentDigit = a_currentDigit - b_currentDigit - borrow; //has to be in for negatives. Will cast to utiny on pass
		if (c_currentDigit < 0/*underflow for subtracting, have to borrow*/) {
			borrow = 1;
			c_currentDigit += 10;
		}
		else {
			borrow = 0; //clear carried number
		}
		wtm(i, c_currentDigit);
	}
}


//public
Decimal_32::~Decimal_32() {
	//does nothing
}

Decimal_32::Decimal_32() {
	for (int i = 0; i < 31; i++) {
		mantissa_[i] = B00000000;
	}
	exponent_ = NORMALEXP_;
}

Decimal_32::Decimal_32(float d, int digits) {
	std::cout << "I will implement this function later\n";
}

Decimal_32::Decimal_32(double d, int digits) {
	std::cout << "I will implement this function later\n";
}

Decimal_32::Decimal_32(std::initializer_list<utiny> mantissa, exp_type exponent, bool signd) {
	exponent_ = exponent;
	for (int i = 0; i < mantissa.size(); i++) {
		wtm(i + DIGITS_ - mantissa.size(), mantissa.begin()[i]);
	}
	if ((DIGITS_ - mantissa.size() - 1) % 2) {
		wtm(DIGITS_ - mantissa.size() - 1, 0);
	}
	for (int i = (DIGITS_ - mantissa.size() + 1/*Offset for sign*/ - 1/*it's an index*/ - 1/*We go one back*/) / 2; i >= 0; i--) {
		mantissa_[i] = B00000000;
	}
	mantissa_[0] = signd ? B00010000 : B00000000;
}

Decimal_32::Decimal_32(const Decimal_32& d) {
	for (int i = 0; i < DIGITBYTES_; i++) {
		mantissa_[i] = d.mantissa_[i]; //obv just copies bytes over
	}
	exponent_ = d.exponent_;
}

Decimal_32::Decimal_32(const std::string& s) {
	bool hasMinus = false;
	size_t size = s.size();
	size_t pointLoc = size - 1;
	int j = DIGITS_ - 1;
	for (int i = size - 1; i >= 0 && j >= 0; i--) {
		if (!i && s[i] == '-') { 
			hasMinus = true;
			continue;
		}
		if (s[i] == '.') {
			if (pointLoc != size - 1) {
				throw std::string("Too Many Decimal Points in String");
			}
			pointLoc = j;
			continue;
		}
		wtm(j, s[i] - '0');
		j--;
	}
	j++;
	while (j--) {
		wtm(j, 0); //TO DO optimize inserting many zeros by doing entire bytes at a time
	}
	wtm(-1, hasMinus); //-1 gets the sign
	exponent_ = NORMALEXP_ - (DIGITS_ - 1) + pointLoc;
}

void Decimal_32::display(void) const {
	auto digitscache = digits();
	int dplocation = (int)exponent_ - NORMALEXP_ + digitscache; 
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
			std::cout << (int)rfm(i);
		}
	}
	else {
		for (int i = DIGITS_ - digitscache; i < DIGITS_; i++) { //more efficient alternative if there is an integrated decimal point
			if (dpiterlocation == i) {
				std::cout << '.';
			}
			std::cout << (int)rfm(i);
		}
	}
}

Decimal_32 Decimal_32::abs(void) const {
	Decimal_32 ret = *this;
	ret.mantissa_[0] = B00001111 & ret.mantissa_[0]; //clear leading sign half-byte
	return ret;
}

void Decimal_32::operator|(Decimal_32& b) {
	//Count avaiable shifting slots for both numbers
	int spacea = 0;
	int spaceb = 0;
	for (int i = 0; i < /*a.*/DIGITS_; i++) {
		if (!rfm(i)) {
			spacea++;
		}
		else {
			break;
		}
	}
	for (int i = 0; i < b.DIGITS_/*Using B here because b is used in the rest of the function*/; i++) {
		if (!rfm(i)) {
			spaceb++;
		}
		else {
			break;
		}
	}
	bool adderbool = ((int)/*a.*/exponent_ - spacea < (int)b.exponent_ - spaceb); //inverted because lesser should be the adder; then it will offset both and add
	Decimal_32& adder = adderbool ? *this : b;
	Decimal_32& addee = adderbool ? b : *this;
	int exponentDifference = diff(/*a.*/exponent_, b.exponent_);
	//offset the adder
	int addeeOffset = min(exponentDifference, (!adderbool) ? spacea : spaceb);
	for (int i = 0; i < addee.DIGITS_ - addeeOffset; i++) { //goes through and shifts
		addee.wtm(i, addee.rfm(i + addeeOffset));
	}
	for (int i = addee.DIGITS_ - addeeOffset; i < addee.DIGITS_; i++) { // flushes after data
		addee.wtm(i, 0); 
	}
	addee.exponent_ -= addeeOffset; //TO DO consider max/min exponents
	int adderOffset = exponentDifference - addeeOffset;
	if (adderOffset) {
		for (int i = adder.DIGITS_ - 1; i >= adderOffset; i--) { //goes through and shifts
			adder.wtm(i, adder.rfm(i - adderOffset));
		}
		for (int i = adderOffset - 1; i >= 0; i--) { // flushes after data
			adder.wtm(i, 0); 
		}
	}
	adder.exponent_ += adderOffset; //TO DO consider max/min exponents
}

void Decimal_32::lshift() {
	int space = lspace();
	if (!space) return;
	for (int i = space; i < DIGITS_; i++) {
		wtm(i - space, rfm(i));
	}
	exponent_ -= space; // TO DO consider what to do when mantissa goes out of range, also rshift
}

void Decimal_32::rshift() {
	int space = rspace();
	if (!space) return;
	for (int i = DIGITS_ - 1; i >= space; i--) {
		wtm(i, rfm(i) - space);
	}
	exponent_ += space;
}

void Decimal_32::lshift(exp_type shift) {
	if (!shift) return;
	for (int i = shift; i < DIGITS_; i++) {
		wtm(i - shift, rfm(i));
	}
	exponent_ -= shift; // TO DO consider what to do when mantissa goes out of range, also rshift
}

void Decimal_32::rshift(exp_type shift) {
	if (!shift) return;
	for (int i = DIGITS_ - 1; i >= shift; i--) {
		wtm(i, rfm(i) - shift);
	}
	exponent_ += shift;
}

exp_type Decimal_32::lspace() {
	int space = 0;
	for (int i = 0; i < /*a.*/DIGITS_; i++) {
		if (!rfm(i)) {
			space++;
		}
		else {
			break;
		}
	}
	return space;
}

exp_type Decimal_32::rspace() {
	int space = 0;
	for (int i = DIGITS_ - 1; i >= 0; i--) {
		if (!rfm(i)) {
			space++;
		}
		else {
			break;
		}
	}
	return space;
}

Decimal_32 operator+(Decimal_32 a, Decimal_32 b) {
	Decimal_32 c;
	a | b; // TO DO: use the output of the space search to quit adding to save time
	c.exponent_ = a.exponent_;

	bool signa = a.mantissa_[0] >> 4;
	bool signb = b.mantissa_[0] >> 4;
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
		utiny a_currentDigit = a.rfm(i);
		utiny b_currentDigit = b.rfm(i);
		utiny c_currentDigit = a_currentDigit + b_currentDigit + carry;
		if (c_currentDigit > 9/*Max number in decimal*/) {
			carry = 1;
			c_currentDigit -= 10;
		}
		else {
			carry = 0; //clear carried number
		}
		c.wtm(i, c_currentDigit); 
	}
	if (carry == 1) { //this means the number is too high for the current exponent, need to scootch everything over
		for (int i = c.DIGITS_ - 1; i >= 0 + 1/*because it is scootching it shouldn't go past 1*/; i--) {
			c.wtm(i, c.rfm(i - 1));
		}
		c.wtm(0, 1);
		if (c.exponent_ < 255) c.exponent_++; //TO DO how should it handle if the exponent is already maxed
	}
	return c;
}

Decimal_32 operator-(Decimal_32 a, Decimal_32 b) {
	Decimal_32 c;
	a | b; // TO DO: use the output of the space search to quit adding to save time
	c.exponent_ = a.exponent_;
	
	bool signa = a.mantissa_[0] >> 4;
	bool signb = b.mantissa_[0] >> 4;
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
	if (b > a) { //wont infinite loop because these get swapped, but -- TO DO: AVOID COMPARISON TWICE (passed flag, seperate function, template, in this function?
		c.pos_subtract(b, a);
		c.negate();
		return c;
	}
	utiny borrow = 0;

	for (int i = c.DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rfm(i);
		utiny b_currentDigit = b.rfm(i);
		int c_currentDigit = a_currentDigit - b_currentDigit - borrow; //has to be in for negatives. Will cast to utiny on pass
		if (c_currentDigit < 0/*underflow for subtracting, have to borrow*/) {
			borrow = 1;
			c_currentDigit += 10 ;
		}
		else {
			borrow = 0; //clear carried number
		}
		c.wtm(i, c_currentDigit);
	}
	//cannot be left with leftover borrow, or it would mean the b was bigger than be, contradicting the original statement
	return c;
}

Decimal_32 operator*(Decimal_32 a, Decimal_32 b)
{
	Decimal_32 c; //TO DO consider not constructing c to not uselessly write
	c.mantissa_[0] = (a.isSigned() == b.isSigned()) << 4; // calculate sign. Can overwrite first digit because it's not written yet.
	a.rshift(); //TO DO figure out more optimal solution
	b.rshift();
	exp_type alspace = a.lspace();
	exp_type blspace = b.lspace();
	bool aisbigger = a.exponent_ > b.exponent_;
	Decimal_32& biggerexp = aisbigger ? a : b;
	Decimal_32& smallerexp = aisbigger ? b : a;
	exp_type bilspace = aisbigger ? alspace : blspace;
	exp_type smlspace = aisbigger ? blspace : alspace;
	exp_type bildigs = biggerexp.DIGITS_ - bilspace; // bi l digits (bigger left digits)
	exp_type smldigs = smallerexp.DIGITS_ - smlspace;
	//exp_type spacediff = diff(bildigs,smldigs); //we cant use biggerexp and smallerexp to decide diff because this is based on space not exponent //delete because not needed?
	//exp_type expdiff = biggerexp.exponent_ - smallerexp.exponent_; //dont use diff macro because we have saved comparison
	int outputdigcount = ((int)(bildigs) + (int)(smldigs)); //gotta account for possible different exponents; must be int so within standards it isn't overflowed
	//TO DO consider way to more accurately figure out if we're going to have the extra digit? 
	utiny carry = 0;
	if (c.DIGITS_ < outputdigcount) { //cutting digits version
		utiny cut = outputdigcount - c.DIGITS_; // TO DO test
		for (int i = biggerexp.DIGITS_ - 1; i >= bilspace; i--) {
			for (int j = smallerexp.DIGITS_ - 1; j >= smlspace; j--) {
				if (i - biggerexp.DIGITS_ + 1 + j + cut <= c.DIGITS_) /* <= because !> */ { //TO DO consider figuring out optimizations to not check every time; only on beginning on for loop resets and until there isn't cut?
					carry = c.atm(i - biggerexp.DIGITS_ + 1 + j + cut, biggerexp.rfm(i) * smallerexp.rfm(j) + carry); //TO DO consider using wtm on first iteration?
				}
			}
			if (i - biggerexp.DIGITS_ + smlspace + cut <= c.DIGITS_) /* >= because !< */ {
				if (carry) {
					c.wtm(i - biggerexp.DIGITS_ /*+ 1 - 1*/ + smlspace + cut, carry); //we can guarantee to wtm, not atm because we should be adding to new ground.
					carry = 0;
				}
			}
		}
	}
	else { //standard version
		for (int i = biggerexp.DIGITS_ - 1; i >= bilspace; i--) {
			for (int j = smallerexp.DIGITS_ - 1; j >= smlspace; j--) {
				carry = c.atm(i - biggerexp.DIGITS_ + 1 + j, biggerexp.rfm(i) * smallerexp.rfm(j) + carry); //TO DO consider using wtm on first iteration?
			}
			if (carry) {
				c.wtm(i - biggerexp.DIGITS_ /*+ 1 - 1*/ + smlspace, carry); //we can guarantee to wtm, not atm because we should be adding to new ground.
				carry = 0;
			}
		}
	}
	c.exponent_ = biggerexp.exponent_ + smallerexp.exponent_ - c.NORMALEXP_; //TO DO figure out what happens with exponents out of range
	return c;
}

Decimal_32 operator/(Decimal_32 a, Decimal_32 b) {
	Decimal_32 c; //TO DO consider not constructing c to not uselessly write
	c.mantissa_[0] = (a.isSigned() == b.isSigned()) << 4;
	a.rshift(); //TO DO figure out more optimal solution
	b.rshift();
	exp_type alspace = a.lspace();
	exp_type blspace = b.lspace();
	bool aisbigger = a.exponent_ > b.exponent_;
	Decimal_32& biggerexp = aisbigger ? a : b;
	Decimal_32& smallerexp = aisbigger ? b : a;
	exp_type bilspace = aisbigger ? alspace : blspace;
	exp_type smlspace = aisbigger ? blspace : alspace;
	exp_type bildigs = biggerexp.DIGITS_ - bilspace; // bi l digits (bigger left digits)
	exp_type smldigs = smallerexp.DIGITS_ - smlspace;
}


bool operator<(Decimal_32 a, Decimal_32 b) { //>, >=, <= are missing comments but they are just variations of these comments
	//two odd lines of logic here, which are IFF'ed.
	//we have smaller and bigger exponents, and if they are in order (smaller, bigger) to (a,b), then this yields true, and other way is false.
	//if the smaller exponent is the smaller number, then this yields true, and the other way is false.
	Decimal_32& biggerexp = a.exponent_ < b.exponent_ ? b : a;
	Decimal_32& smallerexp = a.exponent_ < b.exponent_ ? a : b; //inverted logic not sign bc of edge cases like =
	bool correct = a.exponent_ < b.exponent_;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	//the iterators do NOT LINE UP in the following loops; they couldn't without weirdness. The first is j for biggerexp, then the next two are i's for smallerexp.
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rfm(j) != 0) {
			return correct; //biggerexp is bigger, has leading numbers that smallerexp doesn't have
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) { //check along the aligned digits
		int smallerexpdig = smallerexp.rfm(i); //TO DO consider profiling just making it inline?, also profiling ? < : > method, also use result on *
		int biggerexpdig = biggerexp.rfm(i + exponentdiff);
		if (smallerexpdig != biggerexpdig) {
			return (smallerexpdig < biggerexpdig) ==/*IFF operator*/ correct; //an aligned digit is different. figure out which number is bigger.
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rfm(i) != 0) { //was biggerexp, which was a bug? //TO DO consider making function that indexes faster by taking whole byte
			return !correct; //smallerexp is bigger, has trailing numbers that biggerexp doesn't have
		}
	}
	return false; //isn't related to which is which, they're equal so neither. Could also write return correct because in case it gets here it's equal
}

bool operator>(Decimal_32 a, Decimal_32 b) { //see comments on operator<.
	Decimal_32& biggerexp = a.exponent_ > b.exponent_ ? a : b; //now doing everything in >. Note that edge cases will now swap whether being biggerexp or smaller, but that's intended.
	Decimal_32& smallerexp = a.exponent_ > b.exponent_ ? b : a;
	bool correct = a.exponent_ > b.exponent_; //SWAPPED THIS, THE NEW CORRECT ORDER. EVERYTHING ELSE CHECKS THE SAME; SMALLER SHOULD BE SMALLER, AND VICE VERSA, EXCEPT THE EDGE CASES ARE FLIPPED (see above).

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rfm(j) != 0) {
			return correct;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		int smallerexpdig = smallerexp.rfm(i);
		int biggerexpdig = biggerexp.rfm(i + exponentdiff);
		if (smallerexpdig != biggerexpdig) {
			return (biggerexpdig > smallerexpdig) ==/*IFF operator*/ correct; //this is still the same but I just swapped around the expression because we are using the > operator in this function and this emphasizes it.
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rfm(i) != 0) {
			return !correct;
		}
	}
	return false;
}

bool operator==(Decimal_32 a, Decimal_32 b) {
	Decimal_32& biggerexp = a.exponent_ > b.exponent_ ? a : b; 
	Decimal_32& smallerexp = a.exponent_ > b.exponent_ ? b : a;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rfm(j) != 0) {
			return false; //exclusive leading digits should not be anything besides 0
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		if (smallerexp.rfm(i) != biggerexp.rfm(i + exponentdiff)) {
			return false; //aligned digits have to be equal
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rfm(i) != 0) {
			return false; // exclusive trailing digits should not be anything besides 0
		}
	}
	return true;
}

bool operator!=(Decimal_32 a, Decimal_32 b) {
	Decimal_32& biggerexp = a.exponent_ > b.exponent_ ? a : b; //see ==, returns opposite
	Decimal_32& smallerexp = a.exponent_ > b.exponent_ ? b : a;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rfm(j) != 0) {
			return true;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		if (smallerexp.rfm(i) != biggerexp.rfm(i + exponentdiff)) {
			return true;
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rfm(i) != 0) {
			return true;
		}
	}
	return false;
}

bool operator<=(Decimal_32 a, Decimal_32 b) { //see comments on operator<.
	Decimal_32& biggerexp = a.exponent_ < b.exponent_ ? b : a;
	Decimal_32& smallerexp = a.exponent_ < b.exponent_ ? a : b;
	bool correct = a.exponent_ < b.exponent_;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rfm(j) != 0) {
			return correct;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		int smallerexpdig = smallerexp.rfm(i);
		int biggerexpdig = biggerexp.rfm(i + exponentdiff);
		if (smallerexpdig != biggerexpdig) {
			return (smallerexpdig < biggerexpdig) ==/*IFF operator*/ correct;
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rfm(i) != 0) {
			return !correct;
		}
	}
	return true; //instead, we return true here, because <= should output true on equal.
}

bool operator>=(Decimal_32 a, Decimal_32 b) { //see comments on operator<=.
	Decimal_32& biggerexp = a.exponent_ > b.exponent_ ? a : b;
	Decimal_32& smallerexp = a.exponent_ > b.exponent_ ? b : a;
	bool correct = a.exponent_ > b.exponent_;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rfm(j) != 0) {
			return correct;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		int smallerexpdig = smallerexp.rfm(i);
		int biggerexpdig = biggerexp.rfm(i + exponentdiff);
		if (smallerexpdig != biggerexpdig) {
			return (biggerexpdig > smallerexpdig) == correct;
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rfm(i) != 0) {
			return !correct;
		}
	}
	return true;
}

Decimal_32 Decimal_32::/*Is not a friend function, stays in scope*/operator-() const {
	Decimal_32 ret = *this;
	ret.mantissa_[0] = ((ret.mantissa_[0] ^ B11111111) & B11110000) | ret.mantissa_[0]; //clear leading sign half-byte
	return ret;
}