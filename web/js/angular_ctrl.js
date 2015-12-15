(function() {
  var app = angular.module('angular_app', ['ui.bootstrap', 'ngRoute']);

  app.config(['$routeProvider', function ($routeProvider) {
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

  app.config(['$httpProvider', function ($httpProvider) {  
    $httpProvider.interceptors.push('loading');
  }]);

  app.controller('ModalInstanceCtrl', function ($scope, $uibModalInstance, serial) {

    $scope.serial = serial;
    $scope.accept = function () {
      $uibModalInstance.close(serial.img);
    };

    $scope.reject = function () {
      $uibModalInstance.dismiss('cancel');
    };
  });

  app.controller('camera_ctrl', [ '$http', '$scope', '$uibModal', function ($http, $scope, $uibModal) {
    var store = this;
    store.data = [];

    var tab = 0;

    $scope.serial = {
      value: "",
      img: ""
    }

    $http.get("refresh.live").success(function (response) {
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
      $http.post("val", angular.toJson(jsn)).success(function (response) {
        console.log(response);
      });
    };

    $scope.capture = function() {
      var jsn = {};
      jsn['index'] = tab;
      jsn['filename'] = $scope.serial.value;
      $http.post("capture", angular.toJson(jsn)).success(function (response) {
        $scope.serial.img = response;
        console.log($scope.serial);
        $scope.open();
      });
    };

    $scope.open = function(size) {
      var modalInstance = $uibModal.open({
        animation: true,
        templateUrl: 'modal_window.html',
        controller: 'ModalInstanceCtrl',
        size: size,
        resolve: {
          serial: function() {
            return $scope.serial;
          }
        }
      });

      modalInstance.result.then(function (img) {
        $http.post("save_img", img).success(function (response) {
          console.log(response);
        });
      }, function() {
        $http.post("reject_img", $scope.serial.img).success(function (response) {
          console.log(response);
        });
      });
    };
  }]);
})();