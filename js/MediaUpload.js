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

    item.select('input[name*=comic_image_type]').each(function(radio) {
      radio.observe('click', function(e) { show_insert(radio); });
    });
    
    var type = item.select('input[name*=comic_image_type][checked]').pop();
    if (type) {
      item.select('.filename.new').pop().insert({bottom: new Element('strong').update(type.parentNode.innerHTML.stripTags())});
      show_insert(type);
    }
  });
});
