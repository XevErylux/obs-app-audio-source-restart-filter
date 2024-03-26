# Filter plugin: Reinitialize OBS App audio source

Some audio devices on windows cause a nasty noisy crackling sound after some time (12 minutes or something). See [obsproject/obs-studio#8064](https://github.com/obsproject/obs-studio/issues/8064) for details why it becomes noisy.

According of the posts in this issue Microsoft seems to have a solution ready, but have not yet released it. Currently a lot of streams are living with it, did a workaround with virtual audio cables or stopped using the `App audio source` feature.

I have written a python script plugin for OBS already, but the fading was not very nice. Also the configuration of the specific sources were not possible. You can find it here: https://gist.github.com/XevErylux/96f55bbf1d1397b8863a43f916a534b5

With this new filter plugin you can add it as an audio filter to your affected `App audio source`. Every time you toggle this filter, it will clear and reapply the setting of the window title. This change causes the audio source to reinitialize. Now you have got another 12 minutes of peace. If you combine it with the move plugin, as shown in the video, you can automate this process. You can find the move plugin here: https://obsproject.com/forum/resources/move.913/

## Download

You can find the zip file with the precompiled DLL für OBS x64 for Windows under releases.

## Video

I have attached a video, which demonstrates the automatic reset of the `App audio source`.

https://gist.github.com/assets/148274219/db8c1851-ab21-4162-9311-552c9d5345d7

## Source

In the zip file you can find the precompiled DLL for OBS x64 and the source code. 

