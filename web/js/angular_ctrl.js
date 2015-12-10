(function() {
  var app = angular.module('angular_app', ['ui.bootstrap', 'ngRoute']);

  app.config(['$routeProvider', function($routeProvider) {
    $routeProvider.
      when('/', {
        templateUrl: 'html/camera-template.html',
        controller: 'camera_ctrl',
        controllerAs: 'ctrl'
      }).
      otherwise({
        redirectTo: '/'
      });
  }]);

  app.factory('loading', [function() {  
    var loader = {
        request: function(config) {
            $("#loading").show();
            return config;
        },
        response: function(response) {
            $("#loading").hide();
            return response;
        }
    };
    return loader;
  }]);

  app.config(['$httpProvider', function($httpProvider) {  
    $httpProvider.interceptors.push('loading');
  }]);

  app.controller('camera_ctrl', [ '$http', '$scope', function($http, $scope) {
    var store = this;
    store.data = [];

    var tab = 0;

//    $http.get("json/test.json").success(function(response) {
//      store.data = response;
//    });

    $http.get("refresh.live").success(function(response) {
      store.data = response;
    });

    $scope.changeTab = function(newTab) {
      tab = newTab;
    };

    $scope.valChange = function(name, value) {
      var jsn = {};
      jsn['index'] = tab;
      jsn['key'] = name;
      jsn['value'] = value;
      console.log(jsn);
      $http.post("val", angular.toJson(jsn)).success(function(response) {
        console.log(response);
      });
    };
  }]);
})();
