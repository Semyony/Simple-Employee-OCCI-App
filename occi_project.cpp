
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <occi.h>
#include <iomanip>
#include <string.h>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct Employee {
	int employeeNumber;
	char lastName[50];
	char firstName[50];
	char email[100];
	char phone[50];
	char extension[10];
	char reportsTo[100];
	char jobTitle[50];
	char city[50];
};

int menu(void);
int findEmployee(Connection *conn, int employeeNumber, struct Employee* emp);
void displayEmployee(Connection *conn, struct Employee emp);
void displayAllEmployees(Connection *conn);

//have to implement
void insertEmployee(Connection *conn, struct Employee emp);
void updateEmployee(Connection *conn, int employeeNumber);
void deleteEmployee(Connection *conn, int employeeNumber);
int main() {

	Environment* env = nullptr;
	Connection* conn = nullptr;

	string str;
	string usr = "";
	string pass = "";
	string srv = "";

	try {
		int empnum = 0; // employee number fo WHERE statement
		int choice = 0;
		int found = 0;
		Employee emp;

		env = Environment::createEnvironment(Environment::DEFAULT);
		conn = env->createConnection(usr, pass, srv);

		cout << "********************* HR Menu *********************" << endl;

		do {
			choice = menu();
			switch (choice) {
			case 1:

				cout << "Enter an employee number: ";
				cin >> empnum;
				cout << endl;
				if (findEmployee(conn, empnum, &emp))
					displayEmployee(conn, emp);
				else 
					cout << "Employee " << empnum << " does not exist." << endl << endl;

				break;
			case 2:
				displayAllEmployees(conn);
				break;
			case 3:
				cout << "Employee Number: ";
				cin >> empnum;
				
				if (!findEmployee(conn, empnum, &emp)) {
					emp.employeeNumber = empnum;
					cout << "Last Name: ";
					cin >> emp.lastName;
					cout << "First Name: ";
					cin >> emp.firstName;
					cout << "Email: ";
					cin >> emp.email;
					cout << "extension: ";
					cin >> emp.extension;
					cout << "Job Title: ";
					cin >> emp.jobTitle;
					
					insertEmployee(conn, emp);
					cout << endl << "The new employee is added successfully." << endl << endl;
				}
				else
					cout << endl << "An employee with the same employee number exists." << endl << endl;

				break;
			case 4:
				cout << "Employee Number: ";
				cin >> empnum;
				cout << endl;
				if (findEmployee(conn, empnum, &emp)) {
					updateEmployee(conn, empnum);
					cout << "Employee is updated." << endl << endl;
				}
				else
					cout << "Employee " << empnum << " does not exist." << endl << endl;

				break;
			case 5:
				cout << "Employee Number: ";
				cin >> empnum;
				cout << endl;
				if (findEmployee(conn, empnum, &emp)) {
					deleteEmployee(conn, empnum);
					cout << "Employee is deleted." << endl << endl;
				}
				else
					cout << "Employee " << empnum << " does not exist." << endl << endl;

				break;
			}
		} while (choice != 0);

		
		env->terminateConnection(conn);
		Environment::terminateEnvironment(env);
	}
	catch (SQLException& sqlExcp) {
		cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}

	return 0;
}
	
int menu(void) {
	int choice = 0;

	do {
		cout << "1) Find Employee" << endl;
		cout << "2) Employees Report" << endl;
		cout << "3) Add Employee" << endl;
		cout << "4) Update Employee" << endl;
		cout << "5) Remove Employee" << endl;
		cout << "0) Exit" << endl;
		cout << "Enter an option: ";
		cin >> choice;
		cout << endl;
	} while (choice > 5 && choice < 0);

	return choice;
}


int findEmployee(Connection* conn, int employeeNumber, struct Employee* emp) {
	int found = 0;

	string query = "SELECT employeenumber, lastname, firstname, extension, email, reportsto, jobtitle, officecode FROM employees WHERE employeenumber=";
	query += to_string(employeeNumber);

	Statement* stmt = conn->createStatement();
	ResultSet* rs = stmt->executeQuery(query);

	if (rs->next()) {
		emp->employeeNumber = rs->getInt(1);

		string ln = rs->getString(2);
		strcpy(emp->lastName, ln.c_str());

		string fn = rs->getString(3);
		strcpy(emp->firstName, fn.c_str());

		string extension = rs->getString(4);
		strcpy(emp->extension, extension.c_str());

		string email = rs->getString(5);
		strcpy(emp->email, email.c_str());

		{
			int rt = rs->getInt(6);
			query = "SELECT lastname, firstname FROM employees WHERE employeenumber=";
			query += to_string(rt);
			Statement* st = conn->createStatement();
			ResultSet* rs_rt = st->executeQuery(query);
			if (rs_rt->next()) {
				string ln_r = rs_rt->getString(1);
				string fn_r = rs_rt->getString(2);
				string name = ln_r + " " + fn_r;
				strcpy(emp->reportsTo, name.c_str());
			}
			else {
				strcpy(emp->reportsTo, "\0");
			}
			conn->terminateStatement(st);
		}

		string job = rs->getString(7);
		strcpy(emp->jobTitle, job.c_str());

		{
			int off_code = rs->getInt(8);
			query = "SELECT city, phone FROM offices WHERE officecode=";
			query += to_string(off_code);
			Statement* st = conn->createStatement();
			ResultSet* rs_rt = st->executeQuery(query);
			if (rs_rt->next()) {
				string city = rs_rt->getString(1);
				string phone = rs_rt->getString(2);
				strcpy(emp->city, city.c_str());
				strcpy(emp->phone, phone.c_str());
			}
			conn->terminateStatement(st);
		}

		found = 1;
	}

	conn->terminateStatement(stmt);
	
	return found;
}

void displayEmployee(Connection* conn, struct Employee emp) {
	cout << "employeeNumber = " << emp.employeeNumber << endl;
	cout << "lastName = " << emp.lastName << endl;
	cout << "firstName = " << emp.firstName << endl;
	cout << "email = " << emp.email << endl;
	cout << "phone = " << emp.phone << endl;
	cout << "extension = " << emp.extension << endl;
	cout << "reportsTo = " << emp.reportsTo << endl;
	cout << "jobTitle = " << emp.jobTitle << endl;
	cout << "city = " << emp.city << endl << endl;
}

void displayAllEmployees(Connection* conn) {
	Statement* stmt = conn->createStatement();
	ResultSet* rs = stmt->executeQuery("SELECT employeenumber, lastname, firstname, extension, email, officecode, reportsto FROM employees");
	

	if(!rs->next())
		cout << "There is no employees' information to be displayed." << endl;
	cout << " E      Employee Name 	       Email                   		Phone 	            Ext       Manager        " << endl;
	cout << " ------------------------------------------------------------------------------------------------------------------" << endl;
	while (rs->next()) {
		int en = rs->getInt(1);
		string ln = rs->getString(2);
		string fn = rs->getString(3);
		string name = fn + " " + ln;
		string extension = rs->getString(4);
		string email = rs->getString(5);

		string phone;
		{
			int office = rs->getInt(6);
			string query = "SELECT phone FROM offices WHERE officecode=";
			query += to_string(office);
			Statement* st = conn->createStatement();
			ResultSet* rs_rt = st->executeQuery(query);
			if (rs_rt->next()) {
				phone = rs_rt->getString(1);
			}
		}

		string manager;
		{
			int rt = rs->getInt(7);
			string query = "SELECT lastname, firstname FROM employees WHERE employeenumber=";
			query += to_string(rt);
			Statement* st = conn->createStatement();
			ResultSet* rs_rt = st->executeQuery(query);
			if (rs_rt->next()) {
				string ln_r = rs_rt->getString(1);
				string fn_r = rs_rt->getString(2);
				manager = fn_r + " " + ln_r;
			}
			else {
				manager = "\0";
			}
			conn->terminateStatement(st);
		}

		cout << " " << setw(6) << left << en << " " << setw(22) << left << name << " " << setw(32) << left << email << " " << setw(19) << left << phone << " " << setw(10) << left << extension << manager << endl;
	}
	
	cout << endl;
	
}

void insertEmployee(Connection* conn, struct Employee emp){
	int reportsTo = 1002;
	int officecode = 6;
	string query = "INSERT INTO employees(EMPLOYEENUMBER, LASTNAME, FIRSTNAME, EXTENSION, EMAIL, OFFICECODE, REPORTSTO, JOBTITLE) VALUES("; 
	query += (to_string(emp.employeeNumber) + ", '" + emp.lastName + "', '" + emp.firstName + "', '" + emp.extension + "', '" + emp.email + "', " + to_string(officecode) + ", " + to_string(reportsTo) + ", '" + emp.jobTitle + "')");
	
	Statement* stmt = conn->createStatement();
	stmt->setSQL(query);
	stmt->execute();
	conn->terminateStatement(stmt);
}

void updateEmployee(Connection* conn, int employeeNumber) {
	string ext;
	string query = "UPDATE employees SET EXTENSION = '" ;
	cout << "New Extension: ";
	cin >> ext;
	cout << endl;
	query += ext;
	query += ("' WHERE EMPLOYEENUMBER = " + to_string(employeeNumber));
	Statement* stmt = conn->createStatement();
	stmt->setSQL(query);
	stmt->execute();
	conn->terminateStatement(stmt);

}

void deleteEmployee(Connection* conn, int employeeNumber) {
	string query = ("DELETE FROM employees WHERE EMPLOYEENUMBER =" + to_string(employeeNumber));
	Statement* stmt = conn->createStatement();
	stmt->setSQL(query);
	stmt->execute();
	conn->terminateStatement(stmt);
}
