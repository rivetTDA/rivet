$(document).ready(function() {
	//make all external links open in new windows/tabs
	$.expr[':'].external = function(obj){
		return !obj.href.match(/^mailto\:/) && (obj.hostname != location.hostname) && !obj.href.match(/^javascript\:/) && !obj.href.match(/^$/);
	};
	$('a:external').attr('target', '_blank').attr('rel', 'noopener');
});