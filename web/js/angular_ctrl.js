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

  app.controller('camera_ctrl', [ '$http', function($http) {
    var store = this;
    store.data = [];

    $http.get("json/test.json").success(function(response) {
      store.data = response;
    });

    $scope.valChange = function(name, value) {
      console.log({
        "name": name,
        "value": value
      });
    };
  }]);
})();
