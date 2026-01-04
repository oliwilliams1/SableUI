# Roadmap to 1.0
### Components
- [x] Scroll view
- [x] Tab stack with component exposure/initialisation callback
- [x] Button
- [ ] Sliders
- [x] Input field
- [x] Large text field
- [ ] Input field click for cursor
- [ ] Input field multi-line hover
- [ ] Input field copy/paste support
- [ ] Modal
- [x] Checkbox
- [ ] Progress
- [ ] Dropdown
- [ ] Listbox
- [ ] Keyboard chip
- [ ] Context menu
- [ ] Link?
- [ ] Popover
- [ ] Radio
- [ ] Spinner?
- [ ] Switch
- [ ] Table (contents grid with sortable columns for complex)
- [ ] Toast
- [ ] Tooltip
- [ ] List
- [x] Splitter element (horizontal & vertical)
- [x] Text splitter element
- [ ] Menu bar

### Bug fixes
- [x] Draw window border
- [x] Fix OpenGL context problems
- [x] Triple check refresh things?
- [x] Better/dynamic frame limiting - check "Event processing" in [glfw website](https://www.glfw.org/docs/3.3/input_guide.html)
- [x] Cannot access root element directly in TabWithInitialiser (root=nullptr in Layout())
- [x] Overdraw with scrollview
- [x] Child component state losses
- [x] Scroll bar doesn't update on init / resize

### Features
- [ ] Remove element tree from custom layout targets
- [ ] Make another api for floating components

### Graphics API
- [ ] Abstract shaders
- [ ] Abstract uniforms
- [ ] Abstract drawables
- [ ] Shaderc transpilation
- [ ] Vulkan & Metal support

### Events
- [x] Test keyboard events
- [x] Test scroll events

### Async stuff
- [x] Timeouts
- [x] Timers
- [ ] Animations

### Styling
- [x] Themes
- [x] Text wrap property
- [ ] Inline text colour
- [x] Replace macro styling
- [x] Inline hover styling
- [x] Size and disabled style
- [x] Gap property
- [x] Change texts from subpixel to greyscale for no fringing

### Panels
- [ ] Panel builder mode for user building layouts
- [ ] Panel editor for locking / unlocking panels at runtime
- [ ] Saving panel state across runs

### QOL
- [x] Expose titlebar content api
- [ ] Expose icon setting + program to embed api
- [ ] Allow custom fonts
- [ ] Image lazy-loading
- [ ] Image load-by-buffer
- [x] Props
- [x] Click and drag scroll bar thumb

# Beyond 1.0
- [ ] Docking panels
- [ ] Web support?
- [ ] Mobile support?
- [ ] Touch support
- [ ] Expose graphics & shader api further for cross-platform custom shaders and objects
- [ ] Video player
- [ ] Simple 3d renderer
- [ ] Plugin support
- [ ] External layout script + basic language