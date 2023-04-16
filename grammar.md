# Calculator
The grammar for the language that the calculator can parse.
## Grammar
### With left recursion
*expr* &rarr; *expr* *addop* *term* | *term*<br/>
*addop* &rarr; **+** | **-**<br/>
*term* &rarr; *term* *mulop* *factor* | *factor*<br/>
*mulop* &rarr; **\*** | **/**<br/>
*factor* &rarr; **(** *expr* **)** | **number**

### Without left recursion
We remove left recursion to avoid infinite recursion in our
recursive descent parser.<br/>
*expr* &rarr; *term* *expr‴*<br/>
*expr‴* &rarr; *addop* *term* *expr‴* | ᶓ<br/>
*addop* &rarr; **+** | **-**<br/>
*term* &rarr; *factor* *term‴*<br/>
*term‴* &rarr; *mulop* *factor* *term‴* | ᶓ<br/>
*mulop* &rarr; **\*** | **/**<br/>
*factor* &rarr; **(** *expr* **)** | **number** | **-** **number**