// UI/controllers.js
angular.module('bfApp')
.controller('LoginCtrl', ['$scope','$location','$http',
  function($scope,$location,$http) {
    $scope.user = {user:'', pass:'', showPass:false};
    $scope.error = null;

    $scope.togglePass = () => $scope.user.showPass = !$scope.user.showPass;

    $scope.login = () => {
      $scope.error = null;
      $http.post('/api/login', {
        user: $scope.user.user,
        pass: $scope.user.pass
      })
      .then(resp => {
        if (resp.data.status==='ok') {
          $location.path('/dashboard');
        } else {
          $scope.error = 'Invalid credentials. Try again.';
        }
      })
      .catch(err => {
  if (err.status === 401) {
    $scope.error = 'Invalid credentials. Please enter correct username and password.';
  } else {
    $scope.error = 'Server error – try later.';
  }
});

    };
  }
])
.controller('DashboardCtrl', ['$scope', function($scope){
  $scope.msg = "Welcome to the dashboard!";
}]);
