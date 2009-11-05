var FloatedDivConstructor = Class.create({
  'areas': ["header", "comic", "body", "footer"],
  'generate_html': function(layout) {
    var output = [];
    output.push('<div id="container">');

    var i, il;

    var indent = 1;
    var gi = function() { return "  ".times(indent); }
    var has_whole_sidebar = null;

    var has_layout = {};
    $w('left right').each(function(which) {
      if (layout[which].active) {
        has_layout[which] = [layout[which].start, layout[which].end];
      }
    });
    
    var range = null;
    if (has_layout.left) {
      if (has_layout.right) {
        range = [];

        var all_same = true;
        for (i = 0; i <= 1; ++i) {
          if (has_layout.left[i] != has_layout.right[i]) { all_same = false; break; }
        }

        if (!all_same) {
          $w('left right').each(function(field) {
            if (!has_whole_sidebar) {
              if ((has_layout[field][0] == 0) && (has_layout[field][1] == 3)) {
                has_whole_sidebar = field;
              }
            }
          });
        }

        if (!has_whole_sidebar) {
          range[0] = Math.min(has_layout.left[0], has_layout.right[0]);
          range[1] = Math.max(has_layout.left[1], has_layout.right[1]);
        } else {
          switch (has_whole_sidebar) {
            case 'left': range = has_layout.right; break;
            case 'right': range = has_layout.left; break;
          }
        }
      } else {
        range = has_layout.left;
      }
    } else {
      if (has_layout.right) {
        range = has_layout.right;
      }
    }

    for (i = 0, il = this.areas.length; i < il; ++i) {
      if ((i == 0) && has_whole_sidebar) {
        output.push(gi() + '<div id="whole-sidebar-container">');
        indent++;
        output.push(gi() + '<div id="' + has_whole_sidebar + '-sidebar"><?php echo $' + has_whole_sidebar + '_sidebar ?></div>');
      }
      if (range) {
        if (range[0] == i) {
          output.push(gi() + '<div id="sidebar-container">');
          indent++;
        }
        $w("left right").each(function(field) {
          if (field != has_whole_sidebar) {
            if (has_layout[field]) {
              if (has_layout[field][0] == i) {
                output.push(gi() + '<div id="' + field + '-sidebar"><?php echo $' + field + '_sidebar ?></div>');
              }
            }
          }
        });
      }

      output.push(gi() + '<div id="' + this.areas[i] + '"><?php echo $' + this.areas[i] + ' ?></div>');

      if (range) {
        if (range[1] == i) {
          output.push(gi() + '<br class="clear" />');
          indent--;
          output.push(gi() + '</div>');
        }
      }
      if ((i == 3) && has_whole_sidebar) {
        output.push(gi() + '<br class="clear" />');
        indent--;
        output.push(gi() + '</div>');
      }
    }

    output.push('</div>');

    return output.join("\n");
  },
  'generate_css': function(layout) {
    var output = [];
    var include_container = false;
    
    var area_margins = {};
    var i;
    var myThis = this;
    
    $w('left right').each(function(field) {
      if (layout[field].active) {
        include_container = true;
        output.push('#' + field + '-sidebar { float: ' + field + '; display: inline; width: ' + layout[field].width + 'px }');
        
        for (i = layout[field].start; i <= layout[field].end; ++i) {
          var area = myThis.areas[i];
          if (!area_margins[area]) { area_margins[area] = {}; }
          area_margins[area][field] = layout[field].width;
        }
      }
    });

    for (i = 0; i < this.areas.length; ++i) {
      var area = this.areas[i];
      if (area_margins[area]) {
        var types = [];
        var type;
        for (type in area_margins[area]) {
          types.push("margin-" + type + ": " + area_margins[area][type] + "px"); 
        }
        output.push("#" + area + " { " + types.join("; ") + " }");
      }
    }
    
    if (include_container) {
      output.unshift('#sidebar-container, #whole-sidebar-container { overflow: hidden }');
      output.push('.clear { clear: both }');
    }

    if (layout.body) {
      output.unshift('#container { width: ' + layout.body + 'px }'); 
    }

    return output.join("\n");
  }
});