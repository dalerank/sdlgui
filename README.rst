sdlgui

.. begin_brief_description

NanoGUI is a minimalistic port of NanoGUI claim works with SDL API w/o external dependencies. 
It supports automatic layout generation, stateful C++11 lambdas callbacks, a variety of
useful widget types. Include SDL extensions image/ttf.

.. end_brief_description


Example screenshot
----------------------------------------------------------------------------------------

.. image:: https://github.com/dalerank/sdlgui/raw/master/resources/screenshot1.png
   :alt: Screenshot of Example 1.
   :align: center

Description
----------------------------------------------------------------------------------------

.. _SDL2: https://www.libsdl.org/
.. _SDL2_Image: https://www.libsdl.org/projects/SDL_image/
.. _SDL2_TTF: https://www.libsdl.org/projects/SDL_ttf/

.. end_long_description

Creating widgets
----------------------------------------------------------------------------------------

sdlgui using original NanoGUI style that instantiate widgets, set layout constraints, and
register event callbacks using high-level C++11 code. For instance, the
following two lines from the included example application add a new button to
an existing window `window` and register an event callback.

.. code-block:: cpp

   Button *b = new Button(window, "Plain button");
   b->setCallback([] { cout << "pushed!" << endl; });
   
   //!more simplify

   auto* b = window.button("Plain button",[] { cout << "pushed!" << endl; });

The following lines from the example application create the coupled
slider and text box on the bottom of the second window (see the screenshot).

.. code-block:: cpp

   /* Create an empty panel with a horizontal layout */
   Widget *panel = new Widget(window);
   panel->setLayout(new BoxLayout(BoxLayout::Horizontal, BoxLayout::Middle, 0, 20));

   /* Add a slider and set defaults */
   Slider *slider = new Slider(panel);
   slider->setValue(0.5f);
   slider->setFixedWidth(80);

   /* Add a textbox and set defaults */
   TextBox *textBox = new TextBox(panel);
   textBox->setFixedSize(Vector2i(60, 25));
   textBox->setValue("50");
   textBox->setUnits("%");

   /* Propagate slider changes to the text box */
   slider->setCallback([textBox](float value) {
       textBox->setValue(std::to_string((int) (value * 100)));
   });

   //! more simplify

   /* Create an empty panel with a horizontal layout */
   window.widget()
            .boxlayout(BoxLayout::Horizontal, BoxLayout::Middle, 0, 20))
               /* Add a slider and set defaults */
               .slider(0.5f, [](Slider* obj, float value) {
                     if (auto* textBox = obj->gfind<TextBox>("slider-textbox"))
                        textBox->setValue(std::to_string((int) (value * 100)) );
                  }).withFixedWidth(80).and() 
               /* Add a textbox and set defaults */   
               .textbox("50", "%").withFixedSize(Vector2i(60, 25))    
                  .withId("slider-textbox")

Compiling
----------------------------------------------------------------------------------------

Clone the repository and all dependencies (with ``git clone``),
make directory ``build`` and run ``$build cmake ..``

that generate Makefiles or CMake/Visual Studio project files, and
the rest should just work automatically.

License
----------------------------------------------------------------------------------------

.. begin_license

sdlgui is provided under a BSD-style license that can be found in the LICENSE_
file. By using, distributing, or contributing to this project, you agree to the
terms and conditions of this license.

.. _LICENSE: https://github.com/dalerank/sdlgui/blob/master/LICENSE.txt

Also sdlgui uses Daniel Bruce's `Entypo+ <http://www.entypo.com/>`_ font for the
icons used on various widgets.  This work is licensed under a
`CC BY-SA 4.0 <https://creativecommons.org/licenses/by-sa/4.0/>`_ license.
Commercial entities using NanoGUI should consult the proper legal counsel for
how to best adhere to the attribution clause of the license.

.. end_license
