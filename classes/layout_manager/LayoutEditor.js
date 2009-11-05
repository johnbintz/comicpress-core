var LayoutEditor = Class.create({
  'ratio': 1.25,
  'sections': [
    [ 'header', .1 ],
    [ 'comic',  .3 ],
    [ 'blog',   .5 ],
    [ 'footer', .1 ]
  ],
  'initialize': function(container) {
    this.container = container; 
    
    this.width = this.container.getDimensions()['width'];
    this.height = Math.floor(this.container.getDimensions()['width'] * this.ratio);
    
    var i, il;   
    this.section_handles = [];
    for (i = 0, il = this.sections.length; i < il; ++i) {
      var sh = Math.floor(this.height * this.sections[i][1]);
      var section = new Element("div", { 'style': "height: " + sh + "px", "class": this.sections[i][0] });
      var inside = new Element("div");
      section.insert(inside);
      this.container.insert(section);
      this.section_handles.push(section);
    }
    
    this.sidebar_handles = {};
    var myThis = this;
    $w('left right').each(function(which) {
      var handle = new Element("div", { 'style': 'position: absolute; z-index: 1' });      
      var inside = new Element("div");
      
      var t = new Element("div", { 'class': 'top' });
      var b = new Element("div", { 'class': 'bottom' });
      
      handle.align_bottom = function() {
        var h = handle.getDimensions()['height'];
        b.style.top = h - 5;
      };
      
      handle.insert(t);
      handle.insert(b);      
      handle.insert(inside);
      myThis.container.insert(handle);
      myThis.sidebar_handles[which] = handle;
      
      var generate_handle_move = function(g) {
        return function(e) {
          Event.stop(e);
          
          var cy = handle.viewportOffset()['top'] + document.viewport.getScrollOffsets()['top'];
          var ch = handle.getDimensions()['height'];          
          var ny, nh;
          
          switch(g.className) {
            case 'top':
              ny = e.clientY + document.viewport.getScrollOffsets()['top'];
              nh = ch + (cy - ny);
              break;
            case 'bottom':
              ny = cy; 
              nh = e.clientY - cy + document.viewport.getScrollOffsets()['top'];
              break;
          }
          if (nh < 5) { nh = 5; }
          handle.style.top = ny;
          handle.style.height = nh;

          var i, il;
          var h = 0;
          var closest = { 'top': null, 'bottom': null };
          
          var ty = ny;
          var by = ty + nh;
          
          for (i = 0, il = myThis.section_handles.length; i < il; ++i) {
            var distance = { 'top': null, 'bottom': null };
            distance.top = Math.abs(ty - h);
            h += myThis.section_handles[i].getDimensions()['height'];
            distance.bottom = Math.abs(by - h);
            for (field in closest) {
              var ty = handle.viewportOffset()['top'];
              var by = ny + handle.getDimensions()['height'];
              
              if (closest[field] == null) { closest[field] = [distance[field], i]; }
              if (distance[field] < closest[field][0]) {
                closest[field] = [distance[field], i]; 
              }
            }
          }
          if (closest['bottom'][1] < closest['top'][1]) {
            closest['bottom'][1] = closest['top'][1]; 
          }
          myThis.info.info[which].start = closest['top'][1];
          myThis.info.info[which].end = closest['bottom'][1];
          
          handle.align_bottom();
        };
      };
            
      inside.observe('mousedown', function(e) { Event.stop(e); });
      handle.observe('mousedown', function(e) { Event.stop(e); });
      
      var t_handle = generate_handle_move(t);
      var b_handle = generate_handle_move(b);
      
      t.observe('mousedown', function(e) {
        Event.observe(document.body, 'mousemove', t_handle);
      });
      b.observe('mousedown', function(e) {
        Event.observe(document.body, 'mousemove', b_handle);         
      });
      Event.observe(document.body, 'mouseup', function() {
        Event.stopObserving(document.body, 'mousemove', t_handle);
        Event.stopObserving(document.body, 'mousemove', b_handle);
      });
    });
    
    Event.observe(document.body, 'mouseup', function() {
      myThis.draw_sidebars();
    });
  },
  'register_info': function(info) {
    this.info = info;
    var myThis = this;
    this.info.onChange = function() {
      myThis.draw();
    };
  },
  'draw': function() {
    this.draw_sidebars();  
  },
  'draw_sidebars': function() {
    var myThis = this;
    $w('left right').each(function(field) {
      if (myThis.info.info[field].active) {
        if (myThis.info.info.body > myThis.info.info[field].width) {
          myThis.sidebar_handles[field].show();
          var fi = myThis.info.info[field];
          var t = myThis.section_handles[fi.start].viewportOffset()['top'] + document.viewport.getScrollOffsets()['top'];
          var h = 0;
          var i;
          for (i = fi.start; i <= fi.end; ++i) {
            h += myThis.section_handles[i].getDimensions()['height'];
          }
          var w = Math.floor((fi.width / myThis.info.info.body) * myThis.width);
          var l;
          switch (field) {
            case 'left':
              l = myThis.container.viewportOffset()['left']; break;
            case 'right':
              l = myThis.container.viewportOffset()['left'] + myThis.width - w; break;
          }
          var field_map = { 'top': t, 'left': l, 'width': w, 'height': h };
          for (param in field_map) {
            myThis.sidebar_handles[field].style[param] = field_map[param];
          }
          myThis.sidebar_handles[field].align_bottom();
        } else {
          myThis.sidebar_handles[field].hide();
        }
      } else {
        myThis.sidebar_handles[field].hide();
      }
    });
    myThis.info.do_sidebar_drag();
  }
});


var LayoutInfo = Class.create({
  'info': { 
    'body': 800,
    'left': {
      'active': true, 
      'start':  0,
      'end':    3,
      'width':  200
    }, 
    'right': {
      'active': true, 
      'start':  0,
      'end':    3,
      'width':  175
    }
  },
  'register_form': function(target) {
    var myThis = this;
    $w('left right').each(function(which) {
      var i;
      var get_v = function(v) { return v; }
      for (i in myThis.info[which]) {
        var my_which = get_v(which);
        var f = target.select('input[name=' + which + "-" + i + "]").pop();        
        if (f) {
          switch (i) {
            case 'active':
              f.checked = myThis.info[my_which]['active'];
              f.observe('click', function(e) {
                myThis.info[my_which]['active'] = e.currentTarget.checked;
                myThis.onChange();
              });
              break;
            case 'width':
              f.value = myThis.info[my_which]['width'];
              f.observe('keyup', function(e) {
                myThis.info[my_which]['width'] = e.currentTarget.value.replace(/[^0-9]/, '');
                myThis.onChange();
              });
              break; 
          }  
        }
      }
    });
    
    var body_width = target.select('input[name=body-width]').pop();
    if (body_width) {
      body_width.value = myThis.info.body;
      body_width.observe('keyup', function(e) {
        myThis.info.body = e.currentTarget.value.replace(/[^0-9]/, '');
        myThis.onChange();
      }); 
    }
  },
  'do_sidebar_drag': function() {
    this.onSidebarDrag();
  },
  'onChange': function() {}
});

