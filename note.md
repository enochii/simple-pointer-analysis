## Notes on Anderson

we have 2 main steps, collecting constraints and solving the constraints. 2 things are a little tricky.

- how to dynamically resolve virtual function? for precision, we may not want to treat `this` like other parameters(we DO NOT add constraints, just propagate points-to set). In [this referenced implementation](https://github.com/grievejia/andersen), seems no such handling.
- how to handle function pointer?

Actually, they are both about "dynamic" information, but when we collect constraints(the first step), we only have "static" constraints. Only when we start solving the constraints, can we get the needed information incrementally.

To handle them, we should have some constraints on the fly. But this implementation will focus on provide a prototype first...