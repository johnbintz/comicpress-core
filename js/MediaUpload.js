Event.observe(window, 'load', function() {
  $$('.media-item').each(function(item) {
    item.select('.savesend').each(function(savesend) {
      var button = new Element('input', { type: 'submit', name: 'save', value: 'Save changes' }).addClassName('button');
      savesend.insert({top: button});
    });

    var show_insert = function(t) {
      var b = item.select('input[name*=send]').pop();
      if (b) { b[(t.value == 'none') ? 'show' : 'hide'](); }
    }

    var type = item.select('input[name*=comicpress_management][checked]').pop();
    if (type) {
      item.select('.filename.new').pop().insert({bottom: new Element('div').addClassName('comicpress-is-managing')});
      show_insert(type);
    }
  });
});
