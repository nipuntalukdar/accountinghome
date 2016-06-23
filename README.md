# accountinghome
**Accounting home is a standalone software for home accounting**

I created the project long time back and was in code.google.com before.
It uses Berkeley DB to store the data.

For building the project:
Clone it and issue the below commands in its directory

$ autogen.sh

$ make 

$ make install

Run the program as shown below:

$ homeaccounting

It will show the prompt "COMMAMND>> "

By default the databse is created under your home directory. To override that, you may run the
command as shown below:

$ homeaccounting /path/to/some/other/dir

Use the command show command to get help on the commands available.

One example is shown below:

$homeaccounting 
Using database in directory /home/geet/accounting
COMMAND>> show commands
GET  [ITEM_NAME]  <FROM_DATE>  <TO_DATE>  
MODIFY  ITEM_NAME  [OLD_NAME]  [NEW_NAME]  <FROM_DATE>  <TO_DATE>  
MODIFY  ITEM_PRICE  [ITEM_NAME]  [NEW_PRICE]  <FROM_DATE>  <TO_DATE>  
EXPENSE  <FROM_DATE>  <TO_DATE>  
ADD  ITEM  <DATE>  
ADD  CATEGORY  [CATEGORY_NAME]  
INSERT  [ITEM_NAME]  TO  CATEGORY  [CATAEGORY_NAME]  
DELETE  ITEM  [ITEM_NAME]  FROM  CATEGORY  [CATEGORY_NAME]  
DELETE  ITEM  [ITEM_NAME]  FROM  DATABASE  [FROM_DATE]  [TO_DATE]  
DELETE  CATEGORY  <CATEGORY_NAME>  
SHOW  CATEGORY  <CATEGORY_NAME>  
SHOW  COMMANDS  
SHOW  UNIQUE_ITEMS  
TOTAL  COST  
IMPORT  FROM  [FILENAME]  
EXPORT  TO  [FILENAME]  [FROM_DATE]  <TO_DATE>  
RGET  [REGEX_ITEM]  <FROM_DATE>  <TO_DATE>  
EXIT  
COMMAND>> 


