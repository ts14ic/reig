# reig
Render-"e"gnostic immediate GUI, inspired by what dear ImGUI and the likes do,
but implemented for fun by own means.

So, a Window Manager that can be integrated (theoretically) inside any other 
graphical environment by:
1. Providing a function that draws a colored rectangle
2. Embedding the call to render_all somewhere in the graphical loop.  

Given those, windows with different widgets can be created anywhere in the code, 
no layouting is required (because immediate mode gui, duh). 
The windows can even be textured.

The library can be found in `lib/reig` and two examples of integrations can be found in `GL3-TestBed` and `SDL-TestBed`.  
Videos of it in action can be found in `docs`.
