# Calculator
The grammar for the language that the calculator can parse.
## Grammar
### With left recursion
*exp* &rarr; *exp* *addop* *term* | *term*<br/>
*addop* &rarr; **+** | **-**<br/>
*term* &rarr; *term* *mulop* *factor* | *factor*<br/>
*mulop* &rarr; **\*** | **/**<br/>
*factor* &rarr; **(** *exp* **)** | **number**

### Without left recursion
We remove left recursion to avoid infinite recursion in our
recursive descent parser.<br/>
*exp* &rarr; *term* *exp‴*<br/>
*exp‴* &rarr; *addop* *term* *exp‴* | ᶓ<br/>
*addop* &rarr; **+** | **-**<br/>
*term* &rarr; *factor* *term‴*<br/>
*term‴* &rarr; *mulop* *factor* *term‴* | ᶓ<br/>
*mulop* &rarr; **\*** | **/**<br/>
*factor* &rarr; **(** *exp* **)** | **number** | **-** **number**