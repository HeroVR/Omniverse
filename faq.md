 # **FAQ** #
1. Ctrl + Alt + Shift + O creates an empty MsgBox with no data in my builds.
> You can test Packed build running in dev-mode. Copy devenv.cfg and systemmenu_xxxxx.json to path where Omniverse.Functions.dll exists. You can find systemmenu_xxxxx.json under Omniverse.Function.dll's folder in plugin Omniverse. (xxxxx maybe unity or ue4)

2. What is the difference between Omni in a commercial environment and Omni in a develop environment.
> They seperated since they are two different environments, but the same data struct. So dont worry about modification of actual player's game data.

3. 