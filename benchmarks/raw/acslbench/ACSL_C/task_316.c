/*
 * Task 316 Natural Language:
 * Write an ACSL-annotated C program that models a bank account with operations for deposit,
 * withdrawal, balance inquiry, and initialization. The account has a balance that must always
 * remain between a specified lower and upper limit. The `deposit` function adds a specified amount
 * to the balance, provided it does not exceed the upper limit. The `withdraw` function subtracts a
 * specified amount from the balance, provided it does not go below the lower limit. The
 * `get_balance` function returns the current balance. The `init_account` function initializes a
 * new account with given limits and a starting balance of zero. The main function demonstrates
 * these operations by creating an account, making transactions, and verifying the final balance.
 */

struct account {
  int lower_limit;
  int balance;
  int upper_limit;
};

/*@
predicate valid_account{L}(struct account* account) =
\valid(account) &&
account->lower_limit <= account->balance <= account->upper_limit;
*/

/*@
requires valid_account(account) && 
0<= amount <= account->upper_limit - account -> balance;
assigns account->balance;
ensures valid_account(account) 
&& account->balance == \at(account->balance,Pre) + amount;
*/
void deposit(struct account* account, int amount) {
  account->balance += amount;
}

/*@
requires valid_account(account) && 
0<= amount <= account -> balance -account->lower_limit;
assigns account->balance;
ensures valid_account(account)
&& account->balance == \at(account->balance,Pre) - amount;
*/
void withdraw(struct account* account, int amount) {
  account->balance -= amount;
}

/*@ requires valid_account(account);
  assigns \nothing;
  ensures \result == account -> balance;
*/
int get_balance(struct account* account) { return account ->balance; }

/*@ requires \valid(account) && lower_limit <= 0 <= upper_limit;
  assigns account->balance, account->lower_limit, account->upper_limit;
  ensures valid_account(account) 
  && account->balance == 0 && account->lower_limit == lower_limit 
  && account->upper_limit == upper_limit;
*/
void init_account(struct account* account, int lower_limit, int upper_limit) {
  account->lower_limit = lower_limit;
  account -> upper_limit = upper_limit;
  account->balance = 0;
}

int main () {
  struct account _Account;
  struct account* myAccount = &_Account;
  init_account(myAccount,-100,200);
  deposit(myAccount,10);
  withdraw(myAccount,20);
  int current_balance = get_balance(myAccount);
  //@ assert current_balance == -10;
  return 0;
}
