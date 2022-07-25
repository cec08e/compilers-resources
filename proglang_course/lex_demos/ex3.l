%%
a+     {printf("\nFound a bunch of a's!\n");}
c|d    {printf("\nFound a c or a d.\n");}
"c|d"  {printf("\nFound the string \"c|d\".\n");}
\"     {printf("\nFound a double-quote.\n");}
[1-5]  {printf("\nFound any number 1 through 5.\n");}
[^*&]  {printf("\nFound some character that is not an asterisk or ampersand.\n");}
.      {printf("\nSome other character.\n");}
